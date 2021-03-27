#ifndef _XPC_OBJECTS_FD_H_
#define _XPC_OBJECTS_FD_H_

#import <xpc/objects/base.h>

#include <mach/mach_port.h>

XPC_CLASS_DECL(fd);

struct xpc_fd_s {
	struct xpc_object_s base;
	mach_port_t port;
};

@interface XPC_CLASS_INTERFACE(fd)

@property(readonly) mach_port_t port;

- (instancetype)initWithDescriptor: (int)descriptor;
- (instancetype)initWithPort: (mach_port_t)port;

- (int)instantiateDescriptor;

@end

#endif // _XPC_OBJECTS_FD_H_
