#ifndef _XPC_OBJECTS_SERVICE_INSTANCE_H_
#define _XPC_OBJECTS_SERVICE_INSTANCE_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(service_instance);

struct xpc_service_instance_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(service_instance)

@end

#endif // _XPC_OBJECTS_SERVICE_INSTANCE_H_
