#ifndef _XPC_OBJECTS_ENDPOINT_H_
#define _XPC_OBJECTS_ENDPOINT_H_

#import <xpc/objects/base.h>
#import <xpc/objects/connection.h>

XPC_IGNORE_DUPLICATE_PROTOCOL_PUSH;
XPC_CLASS_DECL(endpoint);
XPC_IGNORE_DUPLICATE_PROTOCOL_POP;

struct xpc_endpoint_s {
	struct xpc_object_s base;
	mach_port_t port;
};

@interface XPC_CLASS_INTERFACE(endpoint)

@property(assign) mach_port_t port;

- (instancetype)initWithConnection: (XPC_CLASS(connection)*)connection;
- (instancetype)initWithPort: (mach_port_t)port;
- (instancetype)initWithPortNoCopy: (mach_port_t)port;

/**
 * Compares `self` with the given endpoint with `self` as the left-hand side and the given endpoint as the right-hand side.
 *
 * @returns 0 if the endpoints are equal, -1 if `self` comes before `rhs`, or 1 if `rhs` comes before `self`.
 */
- (int)compare: (XPC_CLASS(endpoint)*)rhs;

@end

#endif // _XPC_OBJECTS_ENDPOINT_H_
