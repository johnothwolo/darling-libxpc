#ifndef _XPC_OBJECTS_BOOL_H_
#define _XPC_OBJECTS_BOOL_H_

#import <xpc/objects/base.h>

// bools are NOT simple wrapper objects because they are global singletons

XPC_CLASS_DECL(bool);

struct xpc_bool_s {
	struct xpc_object_s base;
	bool value;
};

#undef bool
@interface XPC_CLASS_INTERFACE(bool)

@property(assign) BOOL value;

- (instancetype)initWithValue: (BOOL)value;
+ (instancetype)boolForValue: (BOOL)value;

@end
#define bool _Bool

#endif // _XPC_OBJECTS_BOOL_H_
