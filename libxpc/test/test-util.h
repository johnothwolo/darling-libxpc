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

#ifndef _XPC_TEST_TEST_UTIL_H_
#define _XPC_TEST_TEST_UTIL_H_

#include <xpc/private.h>

#define OBJECT_COUNT 5

static size_t rand_index(size_t count) {
	// not uniform but honestly who cares
	return rand() % count;
};

static const char* rand_key(const char** keys, size_t count) {
	return keys[rand_index(count)];
};

static const uint8_t some_data[] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};

static void create_objects(xpc_object_t* objects, size_t count) {
	if (count >= OBJECT_COUNT) {
		objects[0] = xpc_int64_create(5);
		objects[1] = xpc_string_create("foo");
		objects[2] = xpc_null_create();
		objects[3] = xpc_bool_create(true);
		objects[4] = xpc_data_create(some_data, sizeof(some_data));
	}
};

static void create_keys(const char** keys, size_t count) {
	if (count >= OBJECT_COUNT) {
		keys[0] = "first";
		keys[1] = "second";
		keys[2] = "third";
		keys[3] = "fourth";
		keys[4] = "fifth";
	}
};

static void release_objects(xpc_object_t* objects, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		xpc_release(objects[i]);
	}
};

static void release_keys(const char** keys, size_t count) {
	// no-op
};

#endif // _XPC_TEST_TEST_UTIL_H_
