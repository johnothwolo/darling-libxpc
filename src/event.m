#import <xpc/xpc.h>

// actually, the event publisher is it's own class
typedef xpc_object_t xpc_event_publisher_t;

XPC_EXPORT const char* const _xpc_event_key_name = "XPCEventName";
XPC_EXPORT const char* const _xpc_event_key_stream_name = "XPCEventStreamName";

//
// private C API
//

XPC_EXPORT
void xpc_event_publisher_activate(xpc_event_publisher_t xpub) {

};

XPC_EXPORT
xpc_object_t xpc_event_publisher_create(const char* name, void* some_parameter) {
	return NULL;
};

XPC_EXPORT
int xpc_event_publisher_fire(xpc_event_publisher_t xpub, uint64_t token, xpc_object_t details) {
	return -1;
};

XPC_EXPORT
int xpc_event_publisher_fire_noboost(xpc_event_publisher_t xpub, uint64_t token, xpc_object_t details) {
	return -1;
};

XPC_EXPORT
int xpc_event_publisher_fire_with_reply() {
	// i didn't feel like determining the parameters for this one
	// should be similar to `xpc_event_publisher_fire`
	return -1;
};

XPC_EXPORT
xpc_object_t xpc_event_publisher_fire_with_reply_sync() {
	// should be similar to `xpc_event_publisher_fire_with_reply`
	return NULL;
};

XPC_EXPORT
au_asid_t xpc_event_publisher_get_subscriber_asid(xpc_event_publisher_t xpub, uint64_t token) {
	return -1;
};

XPC_EXPORT
void xpc_event_publisher_set_error_handler(xpc_event_publisher_t xpub, void (^handler)()) {
	// no clue what the parameters for the handler are
	// (probably includes some sort of error parameter, but i have no clue what that is)
};

XPC_EXPORT
void xpc_event_publisher_set_handler(xpc_event_publisher_t xpub, void (^handler)()) {
	// likewise, no clue what the parameters to the handler are
};

XPC_EXPORT
void xpc_event_publisher_set_initial_load_completed_handler_4remoted(xpc_event_publisher_t xpub, void (^handler)()) {
	// once again, no clue about the handler parameters
};

XPC_EXPORT
int xpc_event_publisher_set_subscriber_keepalive(xpc_event_publisher_t xpub, uint64_t token, bool state) {
	return -1;
};

XPC_EXPORT
int xpc_event_stream_check_in() {
	// not a stub
	// just returns 0
	return 0;
};

XPC_EXPORT
int xpc_event_stream_check_in2() {
	// not a stub
	// just returns 0
	return 0;
};

XPC_EXPORT
int xpc_get_event_name(const char* stream, uint64_t token, char* out_name) {
	return -1;
};

XPC_EXPORT
xpc_object_t xpc_copy_event(const char* stream, const char* name) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_event_entitlements(const char* stream, uint64_t token) {
	// returns a dictionary
	return NULL;
};

XPC_EXPORT
void xpc_set_event(const char* stream, const char* name, xpc_object_t descriptor) {

};

XPC_EXPORT
void xpc_set_event_state(const char* stream, uint64_t token, bool state) {

};

XPC_EXPORT
void xpc_set_event_stream_handler(const char* connection_name, dispatch_queue_t queue, void (^handler)(xpc_object_t event)) {

};

XPC_EXPORT
void xpc_set_event_with_flags(const char* stream, const char* name, xpc_object_t descriptor, uint64_t flags) {

};
