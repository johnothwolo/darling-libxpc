#include <os/transaction_private.h>
#include <os/object_private.h>

struct os_transaction_s;
struct os_transaction_extra_vtable_s {};
struct os_transaction_vtable_s {
	void (*_os_obj_xref_dispose)(_os_object_t);
	void (*_os_obj_dispose)(_os_object_t);
	struct os_transaction_extra_vtable_s _os_obj_vtable;
};
extern const struct os_transaction_vtable_s OS_OBJECT_CLASS_SYMBOL(os_transaction) __asm__(OS_OBJC_CLASS_RAW_SYMBOL_NAME(OS_OBJECT_CLASS(os_transaction)));

const struct os_transaction_vtable_s OS_OBJECT_CLASS_SYMBOL(os_transaction) = {
	._os_obj_xref_dispose = NULL,
	._os_obj_dispose = NULL,
	._os_obj_vtable = {},
};

#define OS_TRANSACTION_CLASS (&OS_OBJECT_CLASS_SYMBOL(os_transaction))

struct os_transaction_s {
	_OS_OBJECT_HEADER(
		struct os_transaction_vtable_s* os_obj_isa,
		os_obj_ref_cnt,
		os_obj_xref_cnt
	);
};

os_transaction_t os_transaction_create(const char* transaction_name) {
	return (os_transaction_t)_os_object_alloc_realized(OS_TRANSACTION_CLASS, sizeof(struct os_transaction_s));
};
