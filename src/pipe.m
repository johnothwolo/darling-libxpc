#import <xpc/objects/pipe.h>
#import <xpc/xpc.h>
#import <xpc/private.h>
#define __DISPATCH_INDIRECT__
#import <dispatch/mach_private.h>


XPC_CLASS_SYMBOL_DECL(pipe);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(pipe)

XPC_CLASS_HEADER(pipe);

@end

//
// private C API
//

XPC_EXPORT
void xpc_pipe_invalidate(xpc_pipe_t xpipe) {

};

XPC_EXPORT
xpc_pipe_t xpc_pipe_create(const char* name, int flags) {
	return NULL;
};

XPC_EXPORT
xpc_object_t _od_rpc_call(const char* procname, xpc_object_t payload, xpc_pipe_t (*get_pipe)(bool)) {
	return NULL;
};

XPC_EXPORT
int xpc_pipe_routine_reply(xpc_pipe_t xpipe) {
	return -1;
};

XPC_EXPORT
int xpc_pipe_routine(xpc_pipe_t xpipe, xpc_object_t payload, xpc_object_t* reply) {
	return -1;
};

XPC_EXPORT
int xpc_pipe_try_receive(mach_port_t port, xpc_object_t* out_object, mach_port_t* out_remote_port, boolean_t (*demux)(mach_msg_header_t* request, mach_msg_header_t* reply), mach_msg_size_t max_message_size, int flags) {
	return -1;
};

XPC_EXPORT
int _xpc_pipe_handle_mig(mach_msg_header_t* request, mach_msg_header_t* reply, bool (*demux)(mach_msg_header_t* request, mach_msg_header_t* reply)) {
	return -1;
};

XPC_EXPORT
xpc_pipe_t xpc_pipe_create_from_port(mach_port_t port, mach_port_type_t port_type) {
	// that second parameter is just a guess
	// a good guess, but a guess nonetheless
	return NULL;
};

XPC_EXPORT
int xpc_pipe_receive(mach_port_t port, xpc_object_t* out_msg, bool some_bool) {
	return -1;
};

XPC_EXPORT
int xpc_pipe_routine_async(xpc_pipe_t xpipe, xpc_object_t payload, int some_flags_probably) {
	return -1;
};

XPC_EXPORT
int xpc_pipe_routine_forward(xpc_pipe_t xpipe, xpc_object_t payload) {
	return -1;
};

XPC_EXPORT
int xpc_pipe_routine_with_flags(xpc_pipe_t xpipe, xpc_object_t payload, xpc_object_t* reply, uint64_t flags) {
	return -1;
};

XPC_EXPORT
int xpc_pipe_simpleroutine(xpc_pipe_t xpipe, xpc_object_t payload, int some_flags_probably) {
	return -1;
};
