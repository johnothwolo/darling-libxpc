#import <xpc/xpc.h>
#import <launch.h>
#import <xpc/util.h>
#import <xpc/private.h>

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
	xpc_stub();
	return NULL;
};

XPC_EXPORT
int _launch_service_stats_copy_4ppse_impl(struct some_launch_service_stats_struct* launch_service_stats, int type) {
	// no clue what the structure layout is
	// `type` MUST be 2 or else the function crashes
	// the return type seems to be a status code
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_activate_socket(const char* key, int** fds, size_t* count) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_add_external_service(int handle, const char* path, xpc_object_t overlay) {
	// `overlay` is probably a dictionary
	// no clue what type of value `handle` is other than some integer
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_bootout_user_service_4coresim(const char* name) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
xpc_object_t launch_copy_busy_extension_instances(const char** names, size_t name_count) {
	// returns an array
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t launch_copy_endpoints_properties_for_pid(pid_t pid) {
	// returns a dictionary
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t launch_copy_extension_properties(xpc_connection_t xconn) {
	// returns a dictionary
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t launch_copy_extension_properties_for_pid(pid_t pid) {
	// returns a dictionary
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t launch_copy_properties_for_pid_4assertiond(pid_t pid) {
	// returns a dictionary
	xpc_stub();
	return NULL;
};

XPC_EXPORT
int launch_create_persona(uid_t uid, uint64_t flags) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_destroy_persona(int handle) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_disable_directory(const char* path) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_enable_directory(const char* path) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
void launch_extension_check_in_live_4UIKit(void) {
	xpc_stub();
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
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_get_system_service_enabled(const char* name, bool* out_loaded, bool* out_enabled) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
char* launch_path_for_user_service_4coresim(const char* name) {
	// returns a string that must be freed
	xpc_stub();
	return NULL;
};

XPC_EXPORT const char* launch_perfcheck_property_endpoint_active = "XPCServiceEndpointActive";
XPC_EXPORT const char* launch_perfcheck_property_endpoint_event = "XPCServiceEndpointEvent";
XPC_EXPORT const char* launch_perfcheck_property_endpoint_name = "XPCServiceEndpointName";
XPC_EXPORT const char* launch_perfcheck_property_endpoint_needs_activation = "XPCServiceEndpointNeedsActivation";
XPC_EXPORT const char* launch_perfcheck_property_endpoints = "XPCServiceEndpoints";

XPC_EXPORT
void launch_remove_external_service(const char* name, const char* version, dispatch_queue_t queue, void (^callback)(int error)) {
	xpc_stub();
};

XPC_EXPORT
int launch_service_stats_disable_4ppse(void) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_service_stats_enable_4ppse(void) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
bool launch_service_stats_is_enabled_4ppse() {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_set_service_enabled(const char* name, bool enabled) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int launch_set_system_service_enabled(const char* name, bool enabled) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
char* launch_version_for_user_service_4coresim(const char* name) {
	// returns a string that must be freed
	xpc_stub();
	return NULL;
};

xpc_object_t ld2xpc(launch_data_t data);

static void launch_data_dict_iterator(launch_data_t value, const char* key, void* context) {
	xpc_object_t* result = context;
	xpc_object_t xpc_value = ld2xpc(value);
	xpc_dictionary_set_value(*result, key, xpc_value);
};

// this function isn't present in the official libxpc,
// so i'm guessing that it's actually an inline function
XPC_EXPORT
xpc_object_t ld2xpc(launch_data_t data) {
	xpc_object_t result = NULL;

	switch (launch_data_get_type(data)) {
		case LAUNCH_DATA_DICTIONARY: {
			result = xpc_dictionary_create(NULL, NULL, 0);
			launch_data_dict_iterate(data, launch_data_dict_iterator, &result);
		} break;

		case LAUNCH_DATA_ARRAY: {
			size_t length = launch_data_array_get_count(data);
			result = xpc_array_create(NULL, 0);
			for (size_t i = 0; i < length; ++i) {
				xpc_array_append_value(result, ld2xpc(launch_data_array_get_index(data, i)));
			}
		} break;

		case LAUNCH_DATA_FD: {
			result = xpc_fd_create(launch_data_get_fd(data));
		} break;

		case LAUNCH_DATA_INTEGER: {
			result = xpc_int64_create(launch_data_get_integer(data));
		} break;

		case LAUNCH_DATA_REAL: {
			result = xpc_double_create(launch_data_get_real(data));
		} break;

		case LAUNCH_DATA_BOOL: {
			result = xpc_bool_create(launch_data_get_bool(data));
		} break;

		case LAUNCH_DATA_STRING: {
			result = xpc_string_create(launch_data_get_string(data));
		} break;

		case LAUNCH_DATA_OPAQUE: {
			result = xpc_data_create(launch_data_get_opaque(data), launch_data_get_opaque_size(data));
		} break;

		case LAUNCH_DATA_ERRNO: {
			result = xpc_int64_create(launch_data_get_errno(data));
		} break;

		case LAUNCH_DATA_MACHPORT: {
			// NOTE: i have verified that the contained port does in fact always hold a receive right, so this is the right XPC class to use
			result = xpc_mach_recv_create(launch_data_get_machport(data));
		} break;
	}

	return result;
};

XPC_EXPORT
kern_return_t xpc_call_wakeup(mach_port_t port, int status) {
	xpc_stub();
	return -1;
};
