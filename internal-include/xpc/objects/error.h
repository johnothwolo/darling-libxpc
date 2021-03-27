#ifndef _XPC_OBJECTS_ERROR_H_
#define _XPC_OBJECTS_ERROR_H_

#import <xpc/objects/dictionary.h>

// errors are unique XPC objects: they're basically dictionaries

OS_OBJECT_DECL_SUBCLASS(xpc_error, xpc_object);

struct xpc_error_s {
	struct xpc_dictionary_s base;
};

@interface XPC_CLASS(error) : XPC_CLASS(dictionary) <XPC_CLASS(error)>

@end

#endif // _XPC_OBJECTS_ERROR_H_
