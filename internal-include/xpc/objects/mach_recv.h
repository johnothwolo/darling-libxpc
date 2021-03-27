#ifndef _XPC_OBJECTS_MACH_RECV_H_
#define _XPC_OBJECTS_MACH_RECV_H_

#import <xpc/objects/base.h>
#import <mach/port.h>

XPC_CLASS_DECL(mach_recv);

struct xpc_mach_recv_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(mach_recv)

@end

//
// private C API
//

xpc_object_t xpc_mach_recv_create(mach_port_t recv);
mach_port_t xpc_mach_recv_extract_right(xpc_object_t xrecv);

#endif // _XPC_OBJECTS_MACH_RECV_H_
