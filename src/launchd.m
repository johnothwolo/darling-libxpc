#import <xpc/xpc.h>
#import <launch.h>

//
// private C API
//

struct some_launch_service_stats_struct;

XPC_EXPORT
xpc_object_t _launch_msg2(xpc_object_t request, int type, uint64_t handle) {
	// `request` is probably a dictionary
	// valid values for `type`: 0, 1, 2, 3
	// `handle` can be a normal value or it can be UINT64_MAX (only matters when `type` == )
	// returns a uint64 object
	return NULL;
};

XPC_EXPORT
int _launch_service_stats_copy_4ppse_impl(struct some_launch_service_stats_struct* launch_service_stats, int type) {
	// no clue what the structure layout is
	// `type` MUST be 2 or else the function crashes
	// the return type seems to be a status code
	return -1;
};

XPC_EXPORT
int launch_activate_socket(const char* key, int** fds, size_t* count) {
	return -1;
};

XPC_EXPORT
int launch_add_external_service(int handle, const char* path, xpc_object_t overlay) {
	// `overlay` is probably a dictionary
	// no clue what type of value `handle` is other than some integer
	return -1;
};

XPC_EXPORT
int launch_bootout_user_service_4coresim(const char* name) {
	return -1;
};

XPC_EXPORT
xpc_object_t launch_copy_busy_extension_instances(const char** names, size_t name_count) {
	// returns an array
	return NULL;
};

XPC_EXPORT
xpc_object_t launch_copy_endpoints_properties_for_pid(pid_t pid) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
xpc_object_t launch_copy_extension_properties(xpc_connection_t xconn) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
xpc_object_t launch_copy_extension_properties_for_pid(pid_t pid) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
xpc_object_t launch_copy_properties_for_pid_4assertiond(pid_t pid) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
int launch_create_persona(uid_t uid, uint64_t flags) {
	return -1;
};

XPC_EXPORT
int launch_destroy_persona(int handle) {
	return -1;
};

XPC_EXPORT
int launch_disable_directory(const char* path) {
	return -1;
};

XPC_EXPORT
int launch_enable_directory(const char* path) {
	return -1;
};

XPC_EXPORT
void launch_extension_check_in_live_4UIKit(void) {

};

XPC_EXPORT const char* launch_extension_property_bundle_id = "XPCExtensionBundleIdentifier";
XPC_EXPORT const char* launch_extension_property_host_bundle_id = "XPCExtensionHostBundleIdentifier";
XPC_EXPORT const char* launch_extension_property_host_pid = "XPCExtensionHostPID";
XPC_EXPORT const char* launch_extension_property_path = "XPCExtensionPath";
XPC_EXPORT const char* launch_extension_property_pid = "XPCExtensionPID";
XPC_EXPORT const char* launch_extension_property_version = "XPCExtensionBundleVersion";
XPC_EXPORT const char* launch_extension_property_xpc_bundle = "XPCExtensionXPCBundle";

XPC_EXPORT
int launch_get_service_enabled(const char* name, bool* out_loaded, bool* out_enabled) {
	return -1;
};

XPC_EXPORT
int launch_get_system_service_enabled(const char* name, bool* out_loaded, bool* out_enabled) {
	return -1;
};

XPC_EXPORT
char* launch_path_for_user_service_4coresim(const char* name) {
	// returns a string that must be freed
	return NULL;
};

XPC_EXPORT const char* launch_perfcheck_property_endpoint_active = "XPCServiceEndpointActive";
XPC_EXPORT const char* launch_perfcheck_property_endpoint_event = "XPCServiceEndpointEvent";
XPC_EXPORT const char* launch_perfcheck_property_endpoint_name = "XPCServiceEndpointName";
XPC_EXPORT const char* launch_perfcheck_property_endpoint_needs_activation = "XPCServiceEndpointNeedsActivation";
XPC_EXPORT const char* launch_perfcheck_property_endpoints = "XPCServiceEndpoints";

XPC_EXPORT
void launch_remove_external_service(const char* name, const char* version, dispatch_queue_t queue, void (^callback)(int error)) {

};

XPC_EXPORT
int launch_service_stats_disable_4ppse(void) {
	return -1;
};

XPC_EXPORT
int launch_service_stats_enable_4ppse(void) {
	return -1;
};

XPC_EXPORT
bool launch_service_stats_is_enabled_4ppse() {
	return -1;
};

XPC_EXPORT
int launch_set_service_enabled(const char* name, bool enabled) {
	return -1;
};

XPC_EXPORT
int launch_set_system_service_enabled(const char* name, bool enabled) {
	return -1;
};

XPC_EXPORT
char* launch_version_for_user_service_4coresim(const char* name) {
	// returns a string that must be freed
	return NULL;
};

XPC_EXPORT
xpc_object_t ld2xpc(launch_data_t data) {
	return NULL;
};

XPC_EXPORT
kern_return_t xpc_call_wakeup(mach_port_t port, int status) {
	return -1;
};
