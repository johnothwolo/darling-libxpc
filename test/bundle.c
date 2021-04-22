/**
 * This file is part of Darling.
 *
 * Copyright (C) 2021 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

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
