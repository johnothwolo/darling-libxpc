#import <xpc/objects/bundle.h>
#import <xpc/objects/string.h>
#import <xpc/objects/dictionary.h>
#import <xpc/objects/array.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/private/bundle.h>
#import <xpc/private/plist.h>

#include <Block.h>
#include <sys/stat.h>
#include <dirent.h>

#define INFO_DICT_XPC_SERVICE_KEY "XPCService"

XPC_CLASS_SYMBOL_DECL(bundle);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(bundle)

XPC_CLASS_HEADER(bundle);

+ (BOOL)pathIsBundleRoot: (XPC_CLASS(string)*)path
{
	// frameworks are special
	//
	// in either case, however, we can tell if it's the bundle root if it contains an Info.plist (it's required for all bundles).
	// the difference is just in whether it's directly in the root or in a `Contents` directory.
	if (strcmp(path.pathExtension, "framework") == 0) {
		if (xpc_path_is_file([path stringByAppendingPathComponent: "Info.plist"].UTF8String)) {
			return YES;
		}
	} else {
		if (xpc_path_is_file([path stringByAppendingPathComponent: "Contents/Info.plist"].UTF8String)) {
			return YES;
		}
	}
	return NO;
}

+ (XPC_CLASS(string)*)bundleRootFromPath: (XPC_CLASS(string)*)path
{
	while (![path isEqualToString: "/"]) {
		if ([[self class] pathIsBundleRoot: path]) {
			return path;
		}
		path = [path stringByDeletingLastPathComponent];
	}
	return nil;
}

- (void)dealloc
{
	XPC_THIS_DECL(bundle);

	[this->bundle_path release];
	[this->executable_path release];
	[this->info_dictionary release];
	[this->services release];

	[super dealloc];
}

- (XPC_CLASS(string)*)bundlePath
{
	XPC_THIS_DECL(bundle);
	return this->bundle_path;
}

- (void)setBundlePath: (XPC_CLASS(string)*)bundlePath
{
	XPC_THIS_DECL(bundle);
	[this->bundle_path release];
	this->bundle_path = [bundlePath retain];
}

- (XPC_CLASS(string)*)executablePath
{
	XPC_THIS_DECL(bundle);
	return this->executable_path;
}

- (void)setExecutablePath: (XPC_CLASS(string)*)executablePath
{
	XPC_THIS_DECL(bundle);
	[this->executable_path release];
	this->executable_path = [executablePath retain];
}

- (XPC_CLASS(dictionary)*)infoDictionary
{
	XPC_THIS_DECL(bundle);
	return this->info_dictionary;
}

- (void)setInfoDictionary: (XPC_CLASS(dictionary)*)infoDictionary
{
	XPC_THIS_DECL(bundle);
	[this->info_dictionary release];
	this->info_dictionary = [infoDictionary retain];
}

- (int)error
{
	XPC_THIS_DECL(bundle);
	return this->error;
}

- (void)setError: (int)error
{
	XPC_THIS_DECL(bundle);
	this->error = error;
}

- (BOOL)isFramework
{
	XPC_THIS_DECL(bundle);
	return this->is_framework;
}

- (XPC_CLASS(string)*)infoPath
{
	XPC_CLASS(string)* path = nil;
	if (self.isFramework) {
		path = [self.bundlePath stringByAppendingPathComponent: "Info.plist"];
	} else {
		path = [self.bundlePath stringByAppendingPathComponent: "Contents/Info.plist"];
	}
	return [path stringByResolvingSymlinksInPath];
}

- (instancetype)initWithPath: (XPC_CLASS(string)*)path
{
	if (self = [super init]) {
		XPC_THIS_DECL(bundle);

		self.bundlePath = [[self class] bundleRootFromPath: path];
		if (!self.bundlePath) {
			// if we couldn't find an Info.plist anywhere, let's just blindly trust the user's input
			self.bundlePath = path;
		}

		self.infoDictionary = [XPC_CLASS(dictionary) new];

		this->is_framework = strcmp(self.bundlePath.pathExtension, "framework") == 0;
	}
	return self;
}

- (XPC_CLASS(string)*)pathForResource: (const char*)name ofType: (const char*)type
{
	return [XPC_CLASS(string) stringWithFormat: "%s/%s.%s", self.bundlePath.UTF8String, name, type];
}

- (XPC_CLASS(string)*)pathForSubdirectoryOfType: (xpc_bundle_subdirectory_type_t)type
{
	switch (type) {
		case xpc_bundle_subdirectory_type_xpc_services: {
			return [[self.bundlePath stringByAppendingPathComponent: (self.isFramework) ? "XPCServices" : "Contents/XPCServices"] stringByResolvingSymlinksInPath];
		} break;

		default:
			return nil;
	}
}

- (XPC_CLASS(string)*)resolveExecutableName
{
	XPC_CLASS(string)* result = nil;

	// first try the most common one
	result = [[[self.infoDictionary stringForKey: "CFBundleExecutable"] retain] autorelease];
	if (!result) {
		// then try the other one
		result = [[[self.infoDictionary stringForKey: "NSExecutable"] retain] autorelease];
		if (!result) {
			// finally, if all else fails, try to use the same name as the bundle
			result = [[XPC_CLASS(string) stringWithUTF8String: self.bundlePath.lastPathComponent] stringByDeletingPathExtension];
		}
	}

	return result;
}

- (void)resolveInternal: (xpc_object_t)plist
{
	XPC_THIS_DECL(bundle);

	if (!plist) {
		self.error = xpc_bundle_error_failed_to_read_plist;
		return;
	}

	if (!XPC_CHECK(dictionary, plist)) {
		self.error = xpc_bundle_error_invalid_plist;
		return;
	}

	dispatch_once(&this->resolve_once, ^{
		XPC_CLASS(string)* xpcServicesPath = [self pathForSubdirectoryOfType: xpc_bundle_subdirectory_type_xpc_services];
		DIR* xpcServicesDir = NULL;
		struct dirent* xpcServiceDirent = NULL;
		XPC_CLASS(string)* executableDirTmp = nil;

		self.infoDictionary = XPC_CAST(dictionary, plist);

		// cache the executable path
		executableDirTmp = (self.isFramework) ? self.bundlePath : [self.bundlePath stringByAppendingPathComponent: "Contents/MacOS"];
		self.executablePath = [[executableDirTmp stringByAppendingPathComponent: [self resolveExecutableName].UTF8String] stringByResolvingSymlinksInPath];

		// resolve XPC services
		this->services = [XPC_CLASS(array) new];
		xpcServicesDir = opendir(xpcServicesPath.UTF8String);
		if (xpcServicesDir) {
			while ((xpcServiceDirent = readdir(xpcServicesDir))) {
				@autoreleasepool {
					XPC_CLASS(string)* fullPath = [xpcServicesPath stringByAppendingPathComponent: xpcServiceDirent->d_name];
					if (strcmp(fullPath.pathExtension, "xpc") == 0) {
						XPC_CLASS(bundle)* serviceBundle = [[XPC_CLASS(bundle) alloc] initWithPath: fullPath];
						[serviceBundle resolve];
						if (serviceBundle) {
							[this->services addObject: serviceBundle];
						}
						[serviceBundle release];
					}
				}
			}
			closedir(xpcServicesDir);
		}
	});
}

- (void)resolve
{
	int fd = -1;
	struct stat plist_stat;
	void* buffer = NULL;
	xpc_object_t plist = NULL;

	if (!xpc_path_is_file(self.infoPath.UTF8String)) {
		// no Info.plist; let's continue the resolution with an empty plist
		plist = [XPC_CLASS(dictionary) new];
	} else {
		fd = open(self.infoPath.UTF8String, O_RDONLY);

		if (fd < 0) {
			self.error = xpc_bundle_error_failed_to_read_plist;
			return;
		}

		if (fstat(fd, &plist_stat) < 0) {
			self.error = xpc_bundle_error_failed_to_read_plist;
			close(fd);
			return;
		}

		buffer = malloc(plist_stat.st_size);
		if (!buffer) {
			self.error = xpc_bundle_error_failed_to_read_plist;
			close(fd);
			return;
		}

		if (read(fd, buffer, plist_stat.st_size) != plist_stat.st_size) {
			self.error = xpc_bundle_error_failed_to_read_plist;
			free(buffer);
			close(fd);
			return;
		}

		plist = xpc_create_from_plist(buffer, plist_stat.st_size);

		free(buffer);
		close(fd);
	}

	[self resolveInternal: plist];
	[plist release];
}

- (void)resolveOnQueue: (dispatch_queue_t)resolutionQueue callbackQueue: (dispatch_queue_t)callbackQueue callback: (void(^)(void))callback
{
	int fd = -1;

	if (!xpc_path_is_file(self.infoPath.UTF8String)) {
		// no Info.plist; let's continue the resolution with an empty plist
		callback = Block_copy(callback);
		dispatch_async(resolutionQueue, ^{
			xpc_object_t plist = [XPC_CLASS(dictionary) new];
			[self resolveInternal: plist];
			[plist release];
			dispatch_async(callbackQueue, callback);
			Block_release(callback);
		});
		return;
	}

	fd = open(self.infoPath.UTF8String, O_RDONLY);

	if (fd < 0) {
		self.error = xpc_bundle_error_failed_to_read_plist;
		dispatch_async(callbackQueue, callback);
		return;
	}

	callback = Block_copy(callback);

	xpc_create_from_plist_descriptor(fd, resolutionQueue, ^(xpc_object_t plist) {
		@autoreleasepool {
			close(fd);
			[self resolveInternal: plist];
			dispatch_async(callbackQueue, callback);
			Block_release(callback);
		}
	});
}

@end

//
// private C API
//

XPC_EXPORT
xpc_object_t xpc_bundle_copy_info_dictionary(xpc_object_t xbundle) {
	return [xpc_bundle_get_info_dictionary(xbundle) retain];
};

XPC_EXPORT
char* xpc_bundle_copy_resource_path(xpc_object_t xbundle, const char* name, const char* type) {
	TO_OBJC_CHECKED(bundle, xbundle, bundle) {
		@autoreleasepool {
			XPC_CLASS(string)* path = [bundle pathForResource: name ofType: type];
			return path ? strdup(path.UTF8String) : NULL;
		}
	}
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_bundle_copy_services(xpc_object_t xbundle) {
	// doesn't actually copy the array; only retains it
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_bundle_create(const char* path, unsigned int flags) {
	@autoreleasepool {
		return [[XPC_CLASS(bundle) alloc] initWithPath: [XPC_CLASS(string) stringWithUTF8String: path]];
	}
};

XPC_EXPORT
xpc_object_t xpc_bundle_create_from_origin(unsigned int origin, const char* path) {
	@autoreleasepool {
		return [[XPC_CLASS(bundle) alloc] initWithPath: [XPC_CLASS(string) stringWithUTF8String: path]];
	}
};

XPC_EXPORT
xpc_object_t xpc_bundle_create_main(void) {
	@autoreleasepool {
		char* exec_path = xpc_copy_main_executable_path();
		if (!exec_path) {
			return NULL;
		}
		return [[XPC_CLASS(bundle) alloc] initWithPath: [XPC_CLASS(string) stringWithUTF8StringNoCopy: exec_path freeWhenDone: YES]];
	}
};

XPC_EXPORT
int xpc_bundle_get_error(xpc_object_t xbundle) {
	TO_OBJC_CHECKED(bundle, xbundle, bundle) {
		return bundle.error;
	}
	return 0;
};

XPC_EXPORT
const char* xpc_bundle_get_executable_path(xpc_object_t xbundle) {
	TO_OBJC_CHECKED(bundle, xbundle, bundle) {
		return bundle.executablePath.UTF8String;
	}
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_bundle_get_info_dictionary(xpc_object_t xbundle) {
	TO_OBJC_CHECKED(bundle, xbundle, bundle) {
		return bundle.infoDictionary;
	}
	return NULL;
};

XPC_EXPORT
const char* xpc_bundle_get_path(xpc_object_t xbundle) {
	TO_OBJC_CHECKED(bundle, xbundle, bundle) {
		return bundle.bundlePath.UTF8String;
	}
	return NULL;
};

XPC_EXPORT
uint64_t xpc_bundle_get_property(xpc_object_t xbundle, unsigned int property) {
	// unsure about the return type; it can actually return both integers and pointers
	xpc_stub();
	return 0;
};

XPC_EXPORT
xpc_object_t xpc_bundle_get_xpcservice_dictionary(xpc_object_t xbundle) {
	TO_OBJC_CHECKED(bundle, xbundle, bundle) {
		return [bundle.infoDictionary objectForKey: INFO_DICT_XPC_SERVICE_KEY];
	}
	return NULL;
};

XPC_EXPORT
void xpc_bundle_populate(xpc_object_t xbundle, xpc_object_t info_dictionary, xpc_object_t services_array) {
	xpc_stub();
};

XPC_EXPORT
void xpc_bundle_resolve(xpc_object_t xbundle, dispatch_queue_t callback_queue, void* context, xpc_bundle_resolution_callback_f callback) {
	return xpc_bundle_resolve_on_queue(xbundle, callback_queue, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), context, callback);
};

XPC_EXPORT
void xpc_bundle_resolve_on_queue(xpc_object_t xbundle, dispatch_queue_t callback_queue, dispatch_queue_t resolution_queue, void* context, xpc_bundle_resolution_callback_f callback) {
	TO_OBJC_CHECKED(bundle, xbundle, bundle) {
		@autoreleasepool {
			return [bundle resolveOnQueue: resolution_queue callbackQueue: callback_queue callback: ^{
				callback(bundle, bundle.error, context);
			}];
		}
	}
};

XPC_EXPORT
void xpc_bundle_resolve_sync(xpc_object_t xbundle) {
	TO_OBJC_CHECKED(bundle, xbundle, bundle) {
		@autoreleasepool {
			return [bundle resolve];
		}
	}
};

XPC_EXPORT
void xpc_add_bundle(const char* path, unsigned int flags) {
	xpc_stub();
};

XPC_EXPORT
void xpc_add_bundles_for_domain(xpc_object_t domain, xpc_object_t bundles) {
	xpc_stub();
};
