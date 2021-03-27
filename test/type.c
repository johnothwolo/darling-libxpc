#include "ctest-plus.h"
#include <xpc/private.h>

CTEST(type, get_type) {
	xpc_object_t obj = xpc_int64_create(5);
	ASSERT_EQUAL_PTR(XPC_TYPE_INT64, xpc_get_type(obj));
	xpc_release(obj);
};

CTEST(type, get_type_name) {
	xpc_object_t obj = xpc_int64_create(5);
	ASSERT_STR("OS_xpc_int64", xpc_type_get_name(xpc_get_type(obj)));
	xpc_release(obj);
};
