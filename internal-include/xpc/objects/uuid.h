#ifndef _XPC_OBJECTS_UUID_H_
#define _XPC_OBJECTS_UUID_H_

#import <xpc/objects/base.h>
#include <uuid/uuid.h>

XPC_CLASS_DECL(uuid);

struct xpc_uuid_s {
	struct xpc_object_s base;
	uuid_t value;
};

@interface XPC_CLASS_INTERFACE(uuid)

@property(readonly) uint8_t* bytes;

- (instancetype)initWithBytes: (const uint8_t*)bytes;

@end

#endif // _XPC_OBJECTS_UUID_H_
