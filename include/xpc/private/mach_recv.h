#ifndef _XPC_PRIVATE_MACH_RECV_H_
#define _XPC_PRIVATE_MACH_RECV_H_

#include <xpc/xpc.h>
#include <mach/mach.h>

__BEGIN_DECLS

xpc_object_t xpc_mach_recv_create(mach_port_t recv);
mach_port_t xpc_mach_recv_extract_right(xpc_object_t xrecv);

__END_DECLS

#endif // _XPC_PRIVATE_MACH_RECV_H_
