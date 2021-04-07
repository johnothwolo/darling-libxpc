#ifndef _XPC_OBJECTS_PIPE_H_
#define _XPC_OBJECTS_PIPE_H_

#import <xpc/objects/base.h>

XPC_IGNORE_DUPLICATE_PROTOCOL_PUSH;
XPC_CLASS_DECL(pipe);
XPC_IGNORE_DUPLICATE_PROTOCOL_POP;

struct xpc_pipe_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(pipe)

@end

#endif // _XPC_OBJECTS_PIPE_H_
