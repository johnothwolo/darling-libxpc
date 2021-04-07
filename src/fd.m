#import <xpc/objects/fd.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/serialization.h>

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
		xpc_mach_port_release_send(this->port);
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
		if (xpc_mach_port_retain_send(port) != KERN_SUCCESS) {
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

@implementation XPC_CLASS(fd) (XPCSerialization)

- (BOOL)serializable
{
	return YES;
}

- (NSUInteger)serializationLength
{
	return xpc_serial_padded_length(sizeof(xpc_serial_type_t));
}

+ (instancetype)deserialize: (XPC_CLASS(deserializer)*)deserializer
{
	XPC_CLASS(fd)* result = nil;
	xpc_serial_type_t type = XPC_SERIAL_TYPE_INVALID;
	mach_port_t port = MACH_PORT_NULL;

	if (![deserializer readU32: &type]) {
		goto error_out;
	}
	if (type != XPC_SERIAL_TYPE_FD) {
		goto error_out;
	}

	if (![deserializer readPort: &port type: MACH_MSG_TYPE_PORT_SEND]) {
		goto error_out;
	}

	result = [[[self class] alloc] initWithPort: port];

	return result;

error_out:
	if (result != nil) {
		[result release];
	}
	return nil;
}

- (BOOL)serialize: (XPC_CLASS(serializer)*)serializer
{
	XPC_THIS_DECL(fd);

	if (![serializer writeU32: XPC_SERIAL_TYPE_FD]) {
		goto error_out;
	}

	if (![serializer writePort: this->port type: MACH_MSG_TYPE_COPY_SEND]) {
		goto error_out;
	}

	return YES;

error_out:
	return NO;
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
