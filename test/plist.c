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

#include "xml_plist_data.h"
#include "binary_plist_data.h"

static void verify_result(xpc_object_t result) {
	ASSERT_EQUAL_PTR(XPC_TYPE_DICTIONARY, xpc_get_type(result));
	ASSERT_EQUAL_U(10, xpc_dictionary_get_count(result));
	ASSERT_STR("Foo Bar", xpc_dictionary_get_string(result, "name"));
	ASSERT_EQUAL(500, xpc_dictionary_get_int64(result, "size"));
	ASSERT_TRUE(86.4 == xpc_dictionary_get_double(result, "temperature"));

	xpc_object_t colors = xpc_dictionary_get_array(result, "favorite-colors");
	ASSERT_NOT_NULL(colors);
	ASSERT_EQUAL_U(3, xpc_array_get_count(colors));
	ASSERT_STR("red", xpc_array_get_string(colors, 0));
	ASSERT_STR("green", xpc_array_get_string(colors, 1));
	ASSERT_STR("blue", xpc_array_get_string(colors, 2));

	ASSERT_TRUE(xpc_dictionary_get_bool(result, "Some Boolean"));
	ASSERT_FALSE(xpc_dictionary_get_bool(result, "Some Other Boolean"));

	xpc_object_t data = xpc_dictionary_get_value(result, "some_random_data");
	ASSERT_NOT_NULL(data);
	ASSERT_EQUAL_PTR(XPC_TYPE_DATA, xpc_get_type(data));
	ASSERT_DATA((const uint8_t*)"This is some random data! Tada!", 31, xpc_data_get_bytes_ptr(data), xpc_data_get_length(data));

	ASSERT_STR("look! some cdata like <this> and <that>!", xpc_dictionary_get_string(result, "escapes & comments "));

	xpc_object_t dates = xpc_dictionary_get_array(result, "Dates and Times");
	ASSERT_NOT_NULL(dates);
	// dates aren't verified because they're a little iffy (particularly in binary format)

	xpc_object_t dict = xpc_dictionary_get_dictionary(result, "Look! A nested dictionary!");
	ASSERT_NOT_NULL(dict);
	ASSERT_STR("idk; here's a thing", xpc_dictionary_get_string(dict, "Some INFO"));

	xpc_object_t nested_array = xpc_dictionary_get_array(dict, "And a nested array!");
	ASSERT_NOT_NULL(nested_array);
	ASSERT_EQUAL_U(3, xpc_array_get_count(nested_array));

	xpc_object_t nested_nested_array_1 = xpc_array_get_array(nested_array, 0);
	ASSERT_NOT_NULL(nested_nested_array_1);
	ASSERT_EQUAL_U(2, xpc_array_get_count(nested_nested_array_1));
	ASSERT_STR("Ooh!", xpc_array_get_string(nested_nested_array_1, 0));
	ASSERT_EQUAL(590, xpc_array_get_int64(nested_nested_array_1, 1));

	xpc_object_t nested_nested_array_2 = xpc_array_get_array(nested_array, 1);
	ASSERT_NOT_NULL(nested_nested_array_2);
	ASSERT_EQUAL_U(1, xpc_array_get_count(nested_nested_array_2));
	ASSERT_STR("Foo", xpc_array_get_string(nested_nested_array_2, 0));

	ASSERT_STR("bar", xpc_array_get_string(nested_array, 2));
};

CTEST(plist, create_from_xml_plist) {
	xpc_object_t result = xpc_create_from_plist(xpc_test_xml_plist_data, sizeof(xpc_test_xml_plist_data));
	ASSERT_NOT_NULL(result);
	verify_result(result);
	/*
	char* desc = xpc_copy_description(result);
	CTEST_LOG("%s", desc);
	free(desc);
	*/
};

CTEST(plist, create_from_binary_plist) {
	xpc_object_t result = xpc_create_from_plist(xpc_test_binary_plist_data, sizeof(xpc_test_binary_plist_data));
	ASSERT_NOT_NULL(result);
	verify_result(result);
	/*
	char* desc = xpc_copy_description(result);
	CTEST_LOG("%s", desc);
	free(desc);
	*/
};
