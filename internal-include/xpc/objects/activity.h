#ifndef _XPC_OBJECTS_ACTIVITY_H_
#define _XPC_OBJECTS_ACTIVITY_H_

#import <xpc/objects/base.h>

XPC_IGNORE_DUPLICATE_PROTOCOL_PUSH;
XPC_CLASS_DECL(activity);
XPC_IGNORE_DUPLICATE_PROTOCOL_POP;

struct xpc_activity_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(activity)

@end

#endif // _XPC_OBJECTS_ACTIVITY_H_
