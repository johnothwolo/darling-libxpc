#import <xpc/xpc.h>
#import <mach-o/dyld_priv.h>
#define __DISPATCH_INDIRECT__
#import <dispatch/mach_private.h>
#import <os/voucher_private.h>

XPC_EXPORT
bool _availability_version_check(size_t version_count, dyld_build_version_t* versions) {
	// i'm *pretty* sure the second argument is an array of `dyld_build_version_t`
	return false;
};

XPC_EXPORT
int _system_ios_support_version_copy_string_sysctl(char* out_string) {
	return -1;
};

XPC_EXPORT
bool _system_version_copy_string_plist(char* out_string) {
	return false;
};

XPC_EXPORT
bool _system_version_copy_string_sysctl(char* out_string) {
	return false;
};

XPC_EXPORT uint32_t _system_version_fallback[] = { 10, 15, 0 };

XPC_EXPORT
bool _system_version_parse_string(const char* string, uint32_t* out_version) {
	// `version` is an array of 3 `uint32_t`s (or `int`s)
	return false;
};

XPC_EXPORT
int os_system_version_get_current_version(uint32_t* out_version) {
	// parameter 1's type is a good guess, but i'm unsure
	return -1;
};

XPC_EXPORT
int os_system_version_sim_get_current_host_version(uint32_t* out_version) {
	// same as with `os_system_version_get_current_version`
	return -1;
};

XPC_EXPORT
xpc_object_t _xpc_payload_create_from_mach_msg(dispatch_mach_msg_t msg, int some_flag) {
	return NULL;
};

XPC_EXPORT
void _xpc_spawnattr_pack_string(char* string_but_with_an_offset_of_AEh, uint32_t* offset, size_t* length, const char* string_to_pack) {
	// `string_to_pack` is copied into `string_but_with_an_offset_of_AEh` after offsetting it by 0xae + `*offset`
	// the length of `string_to_pack` (including the null terminator) is added to `*offset` and subtracted from `*length`
};

XPC_EXPORT
void _xpc_spawnattr_pack_string_fragment(char* string_but_with_an_offset_of_AEh, uint32_t* offset, size_t* length, const char* string_to_pack) {
	// same as `_xpc_spawnattr_pack_string`, but doesn't include the null terminator in the math
};

XPC_EXPORT
const char* _xpc_spawnattr_unpack_string(const char* string, size_t length, uint32_t offset) {
	return NULL;
};

XPC_EXPORT
char* _xpc_spawnattr_unpack_strings(char* string_but_with_an_offset_of_AEh, size_t length, uint32_t offset, const char** out_strings, size_t strings_array_size) {
	return NULL;
};

XPC_EXPORT
xpc_object_t place_hold_on_real_loginwindow(void) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_entitlement_for_self(const char* name) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_entitlement_for_token(const char* name, audit_token_t* token) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_entitlements_data_for_token(audit_token_t* token) {
	// returns a data object
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_entitlements_for_pid(pid_t pid) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_entitlements_for_self(void) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_bootstrap(void) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
char* xpc_copy_code_signing_identity_for_token(audit_token_t* token) {
	// returns a string that must be freed
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_domain(void) {
	// returns a dictionary with a single entry:
	// "pid": <pid of current process>
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_extension_sdk_entry() {
	// not a stub
	// just returns NULL
	// probably has parameters, but there's no way to tell what they are
	return NULL;
};

XPC_EXPORT
const char* xpc_exit_reason_get_label(int reason) {
	// `reason` is actually an enum value
	// this function just maps the values with their names
	return NULL;
};

XPC_EXPORT
void xpc_generate_audit_token(pid_t i_think_this_is_a_pid, audit_token_t* token) {

};

XPC_EXPORT
xpc_endpoint_t xpc_get_attachment_endpoint(void) {
	return NULL;
};

struct some_return_remote_hooks_struct;
struct some_remote_hooks_struct;

XPC_EXPORT
struct some_return_remote_hooks_struct* xpc_install_remote_hooks(struct some_remote_hooks_struct* hooks) {
	// the return struct is different from the input struct
	// it's probably some hooks for the caller to call back into libxpc
	return NULL;
};

XPC_EXPORT
void xpc_set_idle_handler() {
	// not a stub
	// just does nothing
	// probably has parameters, but there's no way to tell what they are
};

XPC_EXPORT
bool xpc_test_symbols_exported() {
	// not a stub
	// just returns false
	// probably has parameters, but there's no way to tell what they are
	return false;
};

XPC_EXPORT
void xpc_track_activity(void) {

};

XPC_EXPORT
int xpc_receive_mach_msg(dispatch_mach_msg_t msg, bool end_transaction, voucher_t voucher, xpc_connection_t connection, xpc_object_t* out_object) {
	// parameter 2's purpose is a guess
	return -1;
};

XPC_EXPORT
int xpc_receive_remote_msg(void* data, size_t data_length, bool some_flag, void* something, xpc_connection_t connection, void (^oolCallback)()) {
	// parameter 4 is unknown
	// parameter 6 seems to be a callback, but i'm not sure if it's a raw function or a block (probably a block)
	// i'm also unsure what the parameters for the callback are
	return -1;
};

XPC_EXPORT
void* xpc_make_serialization(xpc_object_t object, size_t* out_serialization_length) {
	return NULL;
};

XPC_EXPORT
void* xpc_make_serialization_with_ool(xpc_object_t object, size_t* out_serialization_length, uint64_t flags) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_create_from_serialization(void* data, size_t data_length) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_create_from_serialization_with_ool(void* data, size_t data_length, void (^oolCallback)()) {
	// same issue with `oolCallback` as in `xpc_receive_remote_msg`
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_create_from_plist(void* data, size_t data_length) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_create_from_plist_descriptor(int fd, dispatch_queue_t queue) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_create_reply_with_format_and_arguments(xpc_object_t original, const char* format, va_list args) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_create_reply_with_format(xpc_object_t original, const char* format, ...) {
	va_list args;
	va_start(args, format);
	xpc_object_t result = xpc_create_reply_with_format_and_arguments(original, format, args);
	va_end(args);
	return result;
};

XPC_EXPORT
xpc_object_t xpc_create_with_format_and_arguments(const char* format, va_list args) {
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_create_with_format(const char* format, ...) {
	va_list args;
	va_start(args, format);
	xpc_object_t result = xpc_create_with_format_and_arguments(format, args);
	va_end(args);
	return result;
};
