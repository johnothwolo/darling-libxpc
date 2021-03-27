#import <xpc/objects/mach_recv.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(mach_recv);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(mach_recv)

XPC_CLASS_HEADER(mach_recv);

@end

//
// private C API
//

XPC_EXPORT
xpc_object_t xpc_mach_recv_create(mach_port_t recv) {
	return NULL;
};

XPC_EXPORT
mach_port_t xpc_mach_recv_extract_right(xpc_object_t xrecv) {
	return MACH_PORT_NULL;
};
