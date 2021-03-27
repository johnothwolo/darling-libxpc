#ifndef _OS_TRANSACTION_PRIVATE_H_
#define _OS_TRANSACTION_PRIVATE_H_

#include <os/object_private.h>

#if OS_OBJECT_USE_OBJC
struct os_transaction_vtable_s {
	_OS_OBJECT_CLASS_HEADER();
};

struct os_transaction_s {
	_OS_OBJECT_HEADER(
		const struct os_transaction_vtable_s* os_obj_isa,
		os_obj_ref_cnt,
		os_obj_xref_cnt
	);
};

@interface OS_OBJECT_CLASS(os_transaction) : OS_OBJECT_CLASS(object)

- (instancetype)initWithName: (const char*)name;

@end

typedef OS_OBJECT_CLASS(os_transaction)* OS_OBJC_INDEPENDENT_CLASS os_transaction_t;
#else
typedef struct os_transaction_s* os_transaction_t;
#endif

os_transaction_t os_transaction_create(const char* transaction_name);

#endif // _OS_TRANSACTION_PRIVATE_H_
