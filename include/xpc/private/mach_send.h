#ifndef _XPC_PRIVATE_MACH_SEND_H_
#define _XPC_PRIVATE_MACH_SEND_H_

#include <xpc/xpc.h>
#include <mach/mach.h>

xpc_object_t xpc_mach_send_create(mach_port_t send);
xpc_object_t xpc_mach_send_create_with_disposition(mach_port_t send, unsigned int disposition);
mach_port_t xpc_mach_send_copy_right(xpc_object_t xsend);

mach_port_t xpc_mach_send_get_right(xpc_object_t xsend);
mach_port_t xpc_mach_send_extract_right(xpc_object_t xsend);

#endif // _XPC_PRIVATE_MACH_SEND_H_
