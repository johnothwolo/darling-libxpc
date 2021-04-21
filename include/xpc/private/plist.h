#ifndef _XPC_PRIVATE_PLIST_H_
#define _XPC_PRIVATE_PLIST_H_

#include <xpc/xpc.h>

__BEGIN_DECLS

xpc_object_t xpc_create_from_plist(const void* _data, size_t length);
void xpc_create_from_plist_descriptor(int fd, dispatch_queue_t queue, void(^callback)(xpc_object_t result));

__END_DECLS

#endif // _XPC_PRIVATE_PLIST_H_
