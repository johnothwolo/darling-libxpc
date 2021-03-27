#import <xpc/objects/fd.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

#include <sys/fileport.h>
#include <mach/mach.h>

XPC_CLASS_SYMBOL_DECL(fd);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(fd)

XPC_CLASS_HEADER(fd);

- (void)dealloc
{
	XPC_THIS_DECL(fd);
	if (this->port != MACH_PORT_NULL) {
		xpc_mach_port_release_right(this->port, MACH_PORT_RIGHT_SEND);
	}
	[super dealloc];
}

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s: port = %d>", xpc_class_name(self), self.port);
	return output;
}

- (mach_port_t)port
{
	XPC_THIS_DECL(fd);
	return this->port;
}

- (instancetype)initWithDescriptor: (int)descriptor
{
	if (self = [super init]) {
		XPC_THIS_DECL(fd);
		if (fileport_makeport(descriptor, &this->port) != KERN_SUCCESS) {
			[self release];
			return nil;
		}
	}
	return self;
}

- (instancetype)initWithPort: (mach_port_t)port
{
	if (self = [super init]) {
		XPC_THIS_DECL(fd);
		if (xpc_mach_port_retain_right(port, MACH_PORT_RIGHT_SEND) != KERN_SUCCESS) {
			[self release];
			return nil;
		}
		this->port = port;
	}
	return self;
}

- (int)instantiateDescriptor
{
	XPC_THIS_DECL(fd);
	return fileport_makefd(this->port);
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(fd);
	return (NSUInteger)this->port;
}

- (instancetype)copy
{
	XPC_THIS_DECL(fd);
	return [[XPC_CLASS(fd) alloc] initWithPort: this->port];
}

@end

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_fd_create(int fd) {
	return [[XPC_CLASS(fd) alloc] initWithDescriptor: fd];
};

XPC_EXPORT
int xpc_fd_dup(xpc_object_t xfd) {
	TO_OBJC_CHECKED(fd, xfd, fd) {
		return [fd instantiateDescriptor];
	}
	return -1;
};

//
// private C API
//

XPC_EXPORT
mach_port_t _xpc_fd_get_port(xpc_object_t xfd) {
	TO_OBJC_CHECKED(fd, xfd, fd) {
		return fd.port;
	}
	return MACH_PORT_NULL;
};
