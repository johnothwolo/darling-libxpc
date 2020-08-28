#ifndef XPC_PRIVATE_H_
#define XPC_PRIVATE_H_

#include <uuid/uuid.h>
#include <xpc/xpc.h>

#ifdef __cplusplus
extern "C" {
#endif

int _xpc_runtime_is_app_sandboxed();

typedef struct _xpc_pipe_s* xpc_pipe_t;

void xpc_pipe_invalidate(xpc_pipe_t pipe);

xpc_pipe_t xpc_pipe_create(const char* name, int flags);

xpc_object_t _od_rpc_call(const char *procname, xpc_object_t payload, xpc_pipe_t (*get_pipe)(bool));

xpc_object_t xpc_create_with_format(const char * format, ...);

xpc_object_t xpc_create_reply_with_format(xpc_object_t original, const char * format, ...);

xpc_object_t xpc_create_from_plist(void *data, size_t size);

void xpc_dictionary_get_audit_token(xpc_object_t, audit_token_t *);
int xpc_pipe_routine_reply(xpc_object_t);
int xpc_pipe_routine(xpc_object_t pipe, void *payload,xpc_object_t *reply);

void xpc_connection_set_target_uid(xpc_connection_t connection, uid_t uid);
void xpc_connection_set_instance(xpc_connection_t connection, uuid_t uid);
void xpc_dictionary_set_mach_send(xpc_object_t object, const char* key, mach_port_t port);

// Completely random. Not sure what the "actual" one is
#define XPC_PIPE_PRIVILEGED 7
#define XPC_PIPE_USE_SYNC_IPC_OVERRIDE 8

#define XPC_PIPE_FLAG_PRIVILEGED XPC_PIPE_PRIVILEGED

xpc_object_t xpc_connection_copy_entitlement_value(xpc_connection_t connection, const char* entitlement);

void xpc_transaction_exit_clean();

#ifdef __cplusplus
}
#endif

#endif

