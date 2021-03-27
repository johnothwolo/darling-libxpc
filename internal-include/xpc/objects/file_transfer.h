#ifndef _XPC_OBJECTS_FILE_TRANSFER_H_
#define _XPC_OBJECTS_FILE_TRANSFER_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(file_transfer);

struct xpc_file_transfer_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(file_transfer)

@end

#endif // _XPC_OBJECTS_FILE_TRANSFER_H_
