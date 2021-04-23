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
#import <xpc/xpc.h>

// this basic testing is related to the basic C API for objects and the base behavior of objects.
// for this purpose, we use int64 objects, as they're simple enough that they should never pose a problem.
// any issues with testing them should come from the xpc_object base class.

// some random non-zero value to start off the integers with
#define INT64_INITIAL_VALUE 5
#define INT64_INITIAL_VALUE_AS_STRING "5"
#define INT64_CLASS_NAME "OS_xpc_int64"

CTEST(base, create) {
	xpc_object_t obj = xpc_int64_create(INT64_INITIAL_VALUE);
	ASSERT_NOT_NULL(obj);
	xpc_release(obj);
};

CTEST(base, refcount) {
	xpc_object_t obj = xpc_int64_create(INT64_INITIAL_VALUE);
	ASSERT_EQUAL_U(1, [obj retainCount]);
	xpc_retain(obj);
	ASSERT_EQUAL_U(2, [obj retainCount]);
	xpc_release(obj);
	ASSERT_EQUAL_U(1, [obj retainCount]);
	xpc_release(obj);
};

// skipped because some XPC objects don't copy themselves
// (e.g. the one we're using, int64, doesn't copy itself)
CTEST_SKIP(base, copy) {
	xpc_object_t obj = xpc_int64_create(INT64_INITIAL_VALUE);
	xpc_object_t copy = xpc_copy(obj);
	ASSERT_NOT_EQUAL_PTR(obj, copy);
	xpc_release(copy);
	xpc_release(obj);
};

CTEST(base, hash) {
	xpc_object_t obj1 = xpc_int64_create(INT64_INITIAL_VALUE);
	xpc_object_t obj2 = xpc_int64_create(INT64_INITIAL_VALUE);
	xpc_object_t obj3 = xpc_int64_create(INT64_INITIAL_VALUE + 1);
	ASSERT_EQUAL_U(xpc_hash(obj1), xpc_hash(obj2));
	ASSERT_NOT_EQUAL_U(xpc_hash(obj1), xpc_hash(obj3));
	xpc_release(obj3);
	xpc_release(obj2);
	xpc_release(obj1);
};

CTEST(base, equality) {
	xpc_object_t obj1 = xpc_int64_create(INT64_INITIAL_VALUE);
	xpc_object_t obj2 = xpc_int64_create(INT64_INITIAL_VALUE);
	xpc_object_t obj3 = xpc_int64_create(INT64_INITIAL_VALUE + 1);
	ASSERT_TRUE(xpc_equal(obj1, obj2));
	ASSERT_FALSE(xpc_equal(obj1, obj3));
	xpc_release(obj3);
	xpc_release(obj2);
	xpc_release(obj1);
};

CTEST(base, description) {
	xpc_object_t obj = xpc_int64_create(INT64_INITIAL_VALUE);
	char* description = xpc_copy_description(obj);
	ASSERT_STR("<" INT64_CLASS_NAME ": " INT64_INITIAL_VALUE_AS_STRING ">", description);
	free(description);
	xpc_release(obj);
};
