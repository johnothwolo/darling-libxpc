#import <xpc/objects/service_instance.h>
#import <xpc/xpc.h>
#import <xpc/endpoint.h>

XPC_CLASS_SYMBOL_DECL(service_instance);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(service_instance)

XPC_CLASS_HEADER(service_instance);

@end

//
// private C API
//

XPC_EXPORT
void xpc_service_instance_dup2(xpc_service_instance_t xinstance, int fd, int new_fd) {

};

XPC_EXPORT
void* xpc_service_instance_get_context(xpc_service_instance_t xinstance) {
	// unsure about the return type, but given the name, it's the most likely type
	return NULL;
};

XPC_EXPORT
pid_t xpc_service_instance_get_host_pid(xpc_service_instance_t xinstance) {
	return -1;
};

XPC_EXPORT
pid_t xpc_service_instance_get_pid(xpc_service_instance_t xinstance) {
	return -1;
};

XPC_EXPORT
int xpc_service_instance_get_type(xpc_service_instance_t xinstance) {
	// unsure about the return type
	return -1;
};

XPC_EXPORT
bool xpc_service_instance_is_configurable(xpc_service_instance_t xinstance) {
	return false;
};

XPC_EXPORT
void xpc_service_instance_run(xpc_service_instance_t xinstance) {

};

XPC_EXPORT
void xpc_service_instance_set_binpref(xpc_service_instance_t xinstance, int binpref) {
	// unsure about `binpref`'s type
};

XPC_EXPORT
void xpc_service_instance_set_context(xpc_service_instance_t xinstance, void* context) {
	// unsure about `context`'s type, but given the name, it's the most likely type
};

XPC_EXPORT
void xpc_service_instance_set_cwd(xpc_service_instance_t xinstance, const char* cwd) {

};

XPC_EXPORT
void xpc_service_instance_set_endpoint(xpc_service_instance_t xinstance, xpc_endpoint_t endpoint) {

};

XPC_EXPORT
void xpc_service_instance_set_environment(xpc_service_instance_t xinstance, xpc_object_t environment) {
	// i'm pretty sure `environment` is a dictionary
};

XPC_EXPORT
void xpc_service_instance_set_finalizer_f(xpc_service_instance_t xinstance, void (*finalizer)(void* context)) {

};

XPC_EXPORT
void xpc_service_instance_set_jetsam_properties(xpc_service_instance_t xinstance, int flags, int priority, int memlimit) {

};

XPC_EXPORT
void xpc_service_instance_set_path(xpc_service_instance_t xinstance, const char* path) {

};

XPC_EXPORT
void xpc_service_instance_set_start_suspended(xpc_service_instance_t xinstance) {
	// takes no other arguments; supposed to set the "start suspended" internal flag
};
