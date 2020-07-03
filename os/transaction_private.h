#ifndef _OS_TRANSACTION_PRIVATE_H_
#define _OS_TRANSACTION_PRIVATE_H_

#include <os/object.h>

OS_OBJECT_DECL_CLASS(os_transaction);

os_transaction_t os_transaction_create(const char* transaction_name);

#endif // _OS_TRANSACTION_PRIVATE_H_
