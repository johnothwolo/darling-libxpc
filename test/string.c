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

#define INITIAL_STRING_LITERAL "foo"
#define INITIAL_STRING_LENGTH 3

CTEST(string, create) {
	xpc_object_t string = xpc_string_create(INITIAL_STRING_LITERAL);
	ASSERT_NOT_NULL(string);
	xpc_release(string);
};

CTEST(string, get_string_ptr) {
	xpc_object_t string = xpc_string_create(INITIAL_STRING_LITERAL);
	ASSERT_NOT_NULL(xpc_string_get_string_ptr(string));
	xpc_release(string);
};

CTEST(string, get_length) {
	xpc_object_t string = xpc_string_create(INITIAL_STRING_LITERAL);
	ASSERT_EQUAL_U(INITIAL_STRING_LENGTH, xpc_string_get_length(string));
	xpc_release(string);
};

CTEST(string, create_with_format) {
	xpc_object_t string = xpc_string_create_with_format("foo %d bar", 5);
	ASSERT_NOT_NULL(string);
	// make sure it formatted it correctly
	ASSERT_STR("foo 5 bar", xpc_string_get_string_ptr(string));
	xpc_release(string);
};

CTEST(string, create_no_copy) {
	xpc_object_t string = xpc_string_create_no_copy(INITIAL_STRING_LITERAL);
	ASSERT_NOT_NULL(string);
	// make sure it actually didn't copy
	ASSERT_EQUAL_PTR(INITIAL_STRING_LITERAL, xpc_string_get_string_ptr(string));
	xpc_release(string);
};

CTEST(string, hash) {
	xpc_object_t string1 = xpc_string_create(INITIAL_STRING_LITERAL);
	xpc_object_t string2 = xpc_string_create(INITIAL_STRING_LITERAL);
	xpc_object_t string3 = xpc_string_create(INITIAL_STRING_LITERAL " bar");
	ASSERT_EQUAL_U(xpc_hash(string1), xpc_hash(string2));
	ASSERT_NOT_EQUAL_U(xpc_hash(string1), xpc_hash(string3));
	xpc_release(string3);
	xpc_release(string2);
	xpc_release(string1);
};

CTEST(string, equality) {
	xpc_object_t string1 = xpc_string_create(INITIAL_STRING_LITERAL);
	xpc_object_t string2 = xpc_string_create(INITIAL_STRING_LITERAL);
	xpc_object_t string3 = xpc_string_create(INITIAL_STRING_LITERAL " bar");
	ASSERT_TRUE(xpc_equal(string1, string2));
	ASSERT_FALSE(xpc_equal(string1, string3));
	xpc_release(string3);
	xpc_release(string2);
	xpc_release(string1);
};

CTEST(string, set_value) {
	xpc_object_t string = xpc_string_create(INITIAL_STRING_LITERAL);
	ASSERT_STR(INITIAL_STRING_LITERAL, xpc_string_get_string_ptr(string));
	_xpc_string_set_value(string, "new string");
	ASSERT_STR("new string", xpc_string_get_string_ptr(string));
	xpc_release(string);
};
