#import <xpc/objects/mach_send.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(mach_send);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(mach_send)

XPC_CLASS_HEADER(mach_send);

@end

//
// private C API
//

XPC_EXPORT
mach_port_t xpc_mach_send_copy_right(xpc_object_t xsend) {
	// retains the send right
	return MACH_PORT_NULL;
};

XPC_EXPORT
xpc_object_t xpc_mach_send_create(mach_port_t send) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_mach_send_create_with_disposition(mach_port_t send, unsigned int disposition) {
	// values for `disposition` are either:
	// - 0x11 - no action taken with send
	// - 0x13 - send is retained
	// - 0x14 - send is `make_send`ed
	return NULL;
};

XPC_EXPORT
mach_port_t xpc_mach_send_get_right(xpc_object_t xsend) {
	// only returns the send right; doesn't retain it
	return MACH_PORT_NULL;
};

mach_port_t xpc_mach_send_extract_right(xpc_object_t xsend) {
	return MACH_PORT_NULL;
};
