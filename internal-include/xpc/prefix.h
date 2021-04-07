#import <os/object.h>

#if __OBJC2__
// Objective-C 2.0 marks `struct objc_class` as unavailable by default.
// we know what we're doing; let's prevent the compiler from complaining.
#undef OBJC2_UNAVAILABLE
#define OBJC2_UNAVAILABLE
#endif

#define XPC_TYPE(name) struct objc_class name
#define XPC_DECL(name) OS_OBJECT_DECL_SUBCLASS(name, xpc_object)
