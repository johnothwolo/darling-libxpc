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
#include "test-util.h"

CTEST_DATA(data) {
	xpc_object_t data;
};

CTEST_SETUP(data) {
	data->data = xpc_data_create(some_data, sizeof(some_data));
};

CTEST_TEARDOWN(data) {
	xpc_release(data->data);
};

CTEST(data, create) {
	xpc_object_t object = xpc_data_create(some_data, sizeof(some_data));
	ASSERT_NOT_NULL(object);
	xpc_release(object);
};

CTEST2(data, length) {
	ASSERT_EQUAL_U(sizeof(some_data), xpc_data_get_length(data->data));
};

CTEST2(data, bytes_ptr) {
	ASSERT_DATA(some_data, sizeof(some_data), xpc_data_get_bytes_ptr(data->data), xpc_data_get_length(data->data));
};

CTEST2(data, bytes) {
	uint8_t copy[sizeof(some_data)];
	size_t copyCount = xpc_data_get_bytes(data->data, copy, 0, sizeof(copy));
	ASSERT_EQUAL_U(sizeof(copy), copyCount);
	ASSERT_DATA(some_data, sizeof(some_data), copy, sizeof(copy));
};

CTEST(data, create_with_dispatch_data) {
	dispatch_data_t ddata = dispatch_data_create(some_data, sizeof(some_data), NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
	xpc_object_t object = xpc_data_create_with_dispatch_data(ddata);
	ASSERT_DATA(some_data, sizeof(some_data), xpc_data_get_bytes_ptr(object), xpc_data_get_length(object));
	xpc_release(object);
	dispatch_release(ddata);
};

CTEST2(data, set_data) {
	const uint8_t new_data[] = { 123, 124, 125, 126, 127, 128, 129, 130 };
	xpc_data_set_value(data->data, new_data, sizeof(new_data));
	ASSERT_DATA(new_data, sizeof(new_data), xpc_data_get_bytes_ptr(data->data), xpc_data_get_length(data->data));
};
