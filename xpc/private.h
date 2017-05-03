#ifndef XPC_PRIVATE_H_
#define XPC_PRIVATE_H_
#include <xpc/xpc.h>

#ifdef __cplusplus
extern "C" {
#endif

int _xpc_runtime_is_app_sandboxed();

typedef struct _xpc_pipe_s* xpc_pipe_t;

void xpc_pipe_invalidate(xpc_pipe_t pipe);

xpc_pipe_t xpc_pipe_create(int name, int arg2);

xpc_object_t _od_rpc_call(const char *procname, xpc_object_t payload, xpc_pipe_t (*get_pipe)(bool));

xpc_object_t xpc_create_with_format(const char * format, ...);

xpc_object_t xpc_create_from_plist(void *data, size_t size);

// Completely random. Not sure what the "actual" one is
#define XPC_PIPE_FLAG_PRIVILEGED 7

#ifdef __cplusplus
}
#endif

#endif

