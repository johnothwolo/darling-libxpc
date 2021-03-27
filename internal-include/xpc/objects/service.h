#ifndef _XPC_OBJECTS_SERVICE_H_
#define _XPC_OBJECTS_SERVICE_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(service);

struct xpc_service_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(service)

@end

#endif // _XPC_OBJECTS_SERVICE_H_
