#import <xpc/objects/base.h>
#import <Foundation/NSZone.h>
#import <Foundation/NSString.h>
#import <objc/runtime.h>
#import <xpc/xpc.h>

// the symbol alias for the base xpc_object class is named differently
XPC_EXPORT struct objc_class _xpc_type_base;
_CREATE_ALIAS(OS_OBJC_CLASS_RAW_SYMBOL_NAME(XPC_CLASS(object)), "__xpc_type_base");

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(object)

XPC_CLASS_HEADER(object);

+ (instancetype)allocWithZone: (NSZone*)zone
{
	return (id)_os_object_alloc_realized([self class], [self _instanceSize]);
}

- (instancetype)init
{
	// we CANNOT call `-[super init]`.
	// libdispatch makes `init` crash on `OS_object`s.
	return self;
}

- (char*)xpcDescription
{
	char* string = NULL;
	asprintf(&string, "<%s: %p>", class_getName([self class]), self);
	return string;
}

- (NSString*)description
{
	Class nsstring = objc_lookUpClass("NSString");
	if (!nsstring) return nil;
	char* desc = self.xpcDescription;
	NSString* string = [nsstring stringWithUTF8String: desc];
	free(desc);
	return string;
}

- (BOOL)isEqual: (id)object
{
	if (![object isKindOfClass: [self class]]) {
		return NO;
	}
	return [self hash] == [object hash];
}

- (instancetype)copy
{
	return [self retain];
}

@end

//
// C API
//

XPC_EXPORT
xpc_object_t (xpc_retain)(xpc_object_t object) {
	return [object retain];
};

XPC_EXPORT
void (xpc_release)(xpc_object_t object) {
	return [object release];
};

XPC_EXPORT
xpc_object_t xpc_copy(xpc_object_t object) {
	return [object copy];
};

XPC_EXPORT
bool xpc_equal(xpc_object_t object1, xpc_object_t object2) {
	return [object1 isEqual: object2];
};

XPC_EXPORT
size_t xpc_hash(xpc_object_t object) {
	return [object hash];
};

XPC_EXPORT
char* xpc_copy_description(xpc_object_t object) {
	return [XPC_CAST(object, object) xpcDescription];
};

//
// private C API
//

XPC_EXPORT
char* xpc_copy_debug_description(xpc_object_t object) {
	// for now
	return xpc_copy_description(object);
};

XPC_EXPORT
char* xpc_copy_short_description(xpc_object_t object) {
	// for now
	return xpc_copy_description(object);
};

XPC_EXPORT
char* xpc_copy_clean_description(xpc_object_t object) {
	// for now
	return xpc_copy_description(object);
};

XPC_EXPORT
char* xpc_inspect_copy_description(xpc_object_t object, xpc_object_t another_object) {
	// returns a string that must be freed
	return NULL;
};

XPC_EXPORT
char* xpc_inspect_copy_description_local(xpc_object_t object, xpc_object_t another_object) {
	// returns a string that must be freed
	return NULL;
};

XPC_EXPORT
char* xpc_inspect_copy_short_description(xpc_object_t object, xpc_object_t another_object) {
	// returns a string that must be freed
	return NULL;
};

XPC_EXPORT
char* xpc_inspect_copy_short_description_local(xpc_object_t object, xpc_object_t another_object) {
	// returns a string that must be freed
	return NULL;
};
