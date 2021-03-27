#ifndef _XPC_OBJECTS_ENDPOINT_H_
#define _XPC_OBJECTS_ENDPOINT_H_

#import <xpc/objects/base.h>
#import <xpc/objects/connection.h>

XPC_CLASS_DECL(endpoint);

struct xpc_endpoint_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(endpoint)

- (instancetype)initWithConnection: (XPC_CLASS(connection)*)connection;

@end

#endif // _XPC_OBJECTS_ENDPOINT_H_
