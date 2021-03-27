#ifndef _XPC_OBJECTS_SERIALIZER_H_
#define _XPC_OBJECTS_SERIALIZER_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(serializer);

struct xpc_serializer_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(serializer)

@end

#endif // _XPC_OBJECTS_SERIALIZER_H_
