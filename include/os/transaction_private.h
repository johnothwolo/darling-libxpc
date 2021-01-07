#ifndef _OS_TRANSACTION_PRIVATE_H_
#define _OS_TRANSACTION_PRIVATE_H_

#include <os/object_private.h>

#if OS_OBJECT_USE_OBJC
OS_OBJECT_DECL_IMPL_CLASS(os_transaction, OS_OBJECT_CLASS(object));
#else
typedef struct os_transaction_s* os_transaction_t;
#endif

os_transaction_t os_transaction_create(const char* transaction_name);

#endif // _OS_TRANSACTION_PRIVATE_H_
