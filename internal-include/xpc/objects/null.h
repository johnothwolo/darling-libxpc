#ifndef _XPC_OBJECTS_NULL_H_
#define _XPC_OBJECTS_NULL_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(null);

struct xpc_null_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(null)

// this API is modeled after NSNull

+ (instancetype)null;

@end

#endif // _XPC_OBJECTS_NULL_H_
