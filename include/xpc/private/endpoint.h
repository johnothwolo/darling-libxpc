#ifndef _XPC_PRIVATE_ENDPOINT_H_
#define _XPC_PRIVATE_ENDPOINT_H_

#include <xpc/xpc.h>
#include <xpc/endpoint.h>

int xpc_endpoint_compare(xpc_endpoint_t xlhs, xpc_endpoint_t xrhs);
mach_port_t xpc_endpoint_copy_listener_port_4sim(xpc_endpoint_t xendpoint);
xpc_endpoint_t xpc_endpoint_create_bs_named(const char* name, uint64_t flags, uint8_t* out_type);
xpc_endpoint_t xpc_endpoint_create_mach_port_4sim(mach_port_t port);

#endif //_XPC_PRIVATE_ENDPOINT_H_
