#import <xpc/objects/connection.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/connection.h>

XPC_CLASS_SYMBOL_DECL(connection);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(connection)

XPC_CLASS_HEADER(connection);

@end

//
// C API
//

XPC_EXPORT
xpc_connection_t xpc_connection_create(const char* name, dispatch_queue_t targetq) {
	return NULL;
};

XPC_EXPORT
xpc_connection_t xpc_connection_create_mach_service(const char* name, dispatch_queue_t targetq, uint64_t flags) {
	return NULL;
};

XPC_EXPORT
xpc_connection_t xpc_connection_create_from_endpoint(xpc_endpoint_t endpoint) {
	return NULL;
};

XPC_EXPORT
void xpc_connection_set_target_queue(xpc_connection_t xconn, dispatch_queue_t targetq) {

};

XPC_EXPORT
void xpc_connection_set_event_handler(xpc_connection_t xconn, xpc_handler_t handler) {

};

XPC_EXPORT
void xpc_connection_suspend(xpc_connection_t xconn) {

};

XPC_EXPORT
void xpc_connection_resume(xpc_connection_t xconn) {

};

XPC_EXPORT
void xpc_connection_send_message(xpc_connection_t xconn, xpc_object_t message) {

};

XPC_EXPORT
void xpc_connection_send_barrier(xpc_connection_t xconn, dispatch_block_t barrier) {

};

XPC_EXPORT
void xpc_connection_send_message_with_reply(xpc_connection_t xconn, xpc_object_t message, dispatch_queue_t replyq, xpc_handler_t handler) {

};

XPC_EXPORT
xpc_object_t xpc_connection_send_message_with_reply_sync(xpc_connection_t xconn, xpc_object_t message) {
	return NULL;
};

XPC_EXPORT
void xpc_connection_cancel(xpc_connection_t xconn) {

};

XPC_EXPORT
const char* xpc_connection_get_name(xpc_connection_t xconn) {
	return NULL;
};

XPC_EXPORT
uid_t xpc_connection_get_euid(xpc_connection_t xconn) {
	return UID_MAX;
};

XPC_EXPORT
gid_t xpc_connection_get_egid(xpc_connection_t xconn) {
	return GID_MAX;
};

XPC_EXPORT
pid_t xpc_connection_get_pid(xpc_connection_t xconn) {
	return -1;
};

XPC_EXPORT
au_asid_t xpc_connection_get_asid(xpc_connection_t xconn) {
	return AU_ASSIGN_ASID;
};

XPC_EXPORT
void xpc_connection_set_context(xpc_connection_t xconn, void* context) {

};

XPC_EXPORT
void* xpc_connection_get_context(xpc_connection_t xconn) {
	return NULL;
};

XPC_EXPORT
void xpc_connection_set_finalizer_f(xpc_connection_t xconn, xpc_finalizer_t finalizer) {

};

XPC_EXPORT
void xpc_connection_set_legacy(xpc_connection_t xconn) {

};

XPC_EXPORT
void xpc_connection_set_privileged(xpc_connection_t xconn) {

};

XPC_EXPORT
void xpc_connection_activate(xpc_connection_t xconn) {

};

XPC_EXPORT
void xpc_connection_set_target_uid(xpc_connection_t xconn, uid_t uid) {

};

XPC_EXPORT
void xpc_connection_set_instance(xpc_connection_t xconn, uuid_t uuid) {

};

XPC_EXPORT
xpc_object_t xpc_connection_copy_entitlement_value(xpc_connection_t xconn, const char* entitlement) {
	return NULL;
};

//
// private C API
//

XPC_EXPORT
void _xpc_connection_set_event_handler_f(xpc_connection_t xconn, void (*handler)(xpc_object_t event, void* context)) {
	// unsure about the parameters to the handler
	// maybe the second parameter to the handler is actually the connection object?
};

XPC_EXPORT
char* xpc_connection_copy_bundle_id(xpc_connection_t xconn) {
	// returns a string that must be freed
	return NULL;
};

XPC_EXPORT
xpc_connection_t xpc_connection_create_listener(const char* name, dispatch_queue_t queue) {
	return NULL;
};

XPC_EXPORT
void xpc_connection_enable_sim2host_4sim(xpc_connection_t xconn) {

};

XPC_EXPORT
void xpc_connection_enable_termination_imminent_event(xpc_connection_t xconn) {

};

XPC_EXPORT
void xpc_connection_get_audit_token(xpc_connection_t xconn, audit_token_t* out_token) {

};

XPC_EXPORT
uint8_t xpc_connection_get_bs_type(xpc_connection_t xconn) {
	return 0;
};

XPC_EXPORT
void xpc_connection_get_instance(xpc_connection_t xconn, uint8_t* out_uuid) {

};

XPC_EXPORT
bool xpc_connection_is_extension(xpc_connection_t xconn) {
	return false;
};

XPC_EXPORT
void xpc_connection_kill(xpc_connection_t xconn, int signal) {

};

XPC_EXPORT

void xpc_connection_send_notification(xpc_connection_t xconn, xpc_object_t details) {

};

XPC_EXPORT
void xpc_connection_set_bootstrap(xpc_connection_t xconn, xpc_object_t bootstrap) {

};

XPC_EXPORT
void xpc_connection_set_bs_type(xpc_connection_t xconn, uint8_t type) {

};

XPC_EXPORT
void xpc_connection_set_event_channel(xpc_connection_t xconn, const char* channel_name) {
	// parameter 2 is a guess
};

XPC_EXPORT
void xpc_connection_set_non_launching(xpc_connection_t xconn, bool non_launching) {

};

XPC_EXPORT
void xpc_connection_set_oneshot_instance(xpc_connection_t xconn, const uint8_t* uuid) {

};

XPC_EXPORT
void xpc_connection_set_qos_class_fallback(xpc_connection_t xconn, dispatch_qos_class_t qos_class) {

};

XPC_EXPORT
void xpc_connection_set_qos_class_floor(xpc_connection_t xconn, dispatch_qos_class_t qos_class, int relative_priority) {

};
