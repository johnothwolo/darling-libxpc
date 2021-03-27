#import <xpc/xpc.h>

XPC_EXPORT
xpc_object_t _xpc_runtime_get_entitlements_data(void) {
	// returns a data object
	return NULL;
};

XPC_EXPORT
xpc_object_t _xpc_runtime_get_self_entitlements(void) {
	// returns a dictionary (parsed from the plist data from `_xpc_runtime_get_entitlements_data`)
	return NULL;
};

XPC_EXPORT
bool _xpc_runtime_is_app_sandboxed(void) {
	return false;
};

XPC_EXPORT
void xpc_main(xpc_connection_handler_t handler) {
	abort();
};

XPC_EXPORT
void xpc_init_services(void) {

};

XPC_EXPORT
void xpc_impersonate_user(void) {
	// not a stub
	// this function just does nothing
};
