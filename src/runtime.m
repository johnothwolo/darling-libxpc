#import <xpc/xpc.h>
#import <xpc/util.h>

XPC_EXPORT
xpc_object_t _xpc_runtime_get_entitlements_data(void) {
	// returns a data object
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t _xpc_runtime_get_self_entitlements(void) {
	// returns a dictionary (parsed from the plist data from `_xpc_runtime_get_entitlements_data`)
	xpc_stub();
	return NULL;
};

XPC_EXPORT
bool _xpc_runtime_is_app_sandboxed(void) {
	xpc_stub();
	return false;
};

XPC_EXPORT
void xpc_main(xpc_connection_handler_t handler) {
	xpc_stub();
	abort();
};

XPC_EXPORT
void xpc_init_services(void) {
	xpc_stub();
};

XPC_EXPORT
void xpc_impersonate_user(void) {
	// not a stub
	// this function just does nothing
};
