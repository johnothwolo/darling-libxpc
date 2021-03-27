#ifndef _XPC_OBJECTS_BUNDLE_H_
#define _XPC_OBJECTS_BUNDLE_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(bundle);

struct xpc_bundle_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(bundle)

@end

#endif // _XPC_OBJECTS_BUNDLE_H_
