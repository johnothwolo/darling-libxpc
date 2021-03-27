#import <xpc/objects/service.h>
#import <xpc/objects/service_instance.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(service);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(service)

XPC_CLASS_HEADER(service);

@end

//
// private C API
//

XPC_EXPORT
void xpc_service_attach(xpc_service_t xservice, bool run, bool kill) {

};

XPC_EXPORT
xpc_service_t xpc_service_create(int flags, const char* name, uint64_t handle, dispatch_queue_t queue) {
	return NULL;
};

XPC_EXPORT
xpc_service_t xpc_service_create_from_specifier(const char* specifier, dispatch_queue_t queue) {
	return NULL;
};

XPC_EXPORT
const char* xpc_service_get_rendezvous_token(xpc_service_t xservice) {
	return NULL;
};

XPC_EXPORT
void xpc_service_kickstart(xpc_service_t xservice, bool suspended, bool kill) {

};

XPC_EXPORT
void xpc_service_set_attach_handler(xpc_service_t xservice, void (^handler)(xpc_service_instance_t xinstance)) {

};

XPC_EXPORT
void _xpc_service_last_xref_cancel(xpc_service_t xservice) {

};

XPC_EXPORT
void xpc_handle_service() {
	// i have no clue what the types for the 3 arguments to this function are
};

XPC_EXPORT
void xpc_handle_subservice() {
	// same as for `xpc_handle_service`
};
