#ifndef _XPC_OBJECTS_SHMEM_H_
#define _XPC_OBJECTS_SHMEM_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(shmem);

struct xpc_shmem_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(shmem)

@end

#endif // _XPC_OBJECTS_SHMEM_H_
