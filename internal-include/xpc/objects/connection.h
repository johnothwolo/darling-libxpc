#ifndef _XPC_OBJECTS_CONNECTION_H_
#define _XPC_OBJECTS_CONNECTION_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(connection);

struct xpc_connection_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(connection)

@end

#endif // _XPC_OBJECTS_CONNECTION_H_
