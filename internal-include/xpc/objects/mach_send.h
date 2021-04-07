#ifndef _XPC_OBJECTS_MACH_SEND_H_
#define _XPC_OBJECTS_MACH_SEND_H_

#import <xpc/objects/base.h>
#import <mach/mach_port.h>

XPC_CLASS_DECL(mach_send);

struct xpc_mach_send_s {
	struct xpc_object_s base;
	mach_port_t port;
};

@interface XPC_CLASS_INTERFACE(mach_send)

@property(readonly) mach_port_t port;

- (instancetype)initWithPort: (mach_port_t)port;
- (instancetype)initWithPortNoCopy: (mach_port_t)port;
- (instancetype)initWithPort: (mach_port_t)port disposition: (mach_msg_type_name_t)disposition;

- (mach_port_t)extractPort;
- (mach_port_t)copyPort;

@end

//
// private C API
//

xpc_object_t xpc_mach_send_create(mach_port_t send);
mach_port_t xpc_mach_send_copy_right(xpc_object_t xsend);
xpc_object_t xpc_mach_send_create_with_disposition(mach_port_t send, unsigned int disposition);
mach_port_t xpc_mach_send_get_right(xpc_object_t xsend);
mach_port_t xpc_mach_send_extract_right(xpc_object_t xsend);

#endif // _XPC_OBJECTS_MACH_SEND_H_
