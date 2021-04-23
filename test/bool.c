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

#undef bool

CTEST(bool, global) {
	xpc_object_t trueObj = xpc_bool_create(true);
	xpc_object_t falseObj = xpc_bool_create(false);

	ASSERT_EQUAL_U(true, xpc_bool_get_value(trueObj));
	ASSERT_EQUAL_U(false, xpc_bool_get_value(falseObj));

	// they should be globals
	ASSERT_EQUAL_PTR(XPC_BOOL_TRUE, trueObj);
	ASSERT_EQUAL_PTR(XPC_BOOL_FALSE, falseObj);

	xpc_release(falseObj);
	xpc_release(trueObj);
};

CTEST(bool, distinct) {
	xpc_object_t obj = _xpc_bool_create_distinct(true);

	ASSERT_EQUAL_U(true, xpc_bool_get_value(obj));

	// it should be a non-global
	ASSERT_NOT_EQUAL_PTR(XPC_BOOL_TRUE, obj);

	_xpc_bool_set_value(obj, false);

	ASSERT_EQUAL_U(false, xpc_bool_get_value(obj));

	xpc_release(obj);
};
