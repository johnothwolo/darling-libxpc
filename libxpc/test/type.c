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
