#include "ctest-plus.h"
#include <xpc/private.h>

#ifndef SAMPLE_FRAMEWORK_PATH
	#error Define SAMPLE_FRAMEWORK_PATH for bundle.c
#endif

#ifndef SAMPLE_FRAMEWORK_EXECUTABLE_PATH
	#error Define SAMPLE_FRAMEWORK_EXECUTABLE_PATH for bundle.c
#endif

CTEST(bundle, create_from_framework) {
	xpc_object_t bundle = xpc_bundle_create(SAMPLE_FRAMEWORK_PATH, 0);
	ASSERT_NOT_NULL(bundle);

	xpc_bundle_resolve_sync(bundle);
	ASSERT_EQUAL(0, xpc_bundle_get_error(bundle));

	ASSERT_STR(SAMPLE_FRAMEWORK_EXECUTABLE_PATH, xpc_bundle_get_executable_path(bundle));
};
