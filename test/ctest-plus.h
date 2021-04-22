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

#ifndef _CTEST_PLUS_H_
#define _CTEST_PLUS_H_

#include <ctest.h>

// convenience header that adds additional macros for ctest

#define ASSERT_EQUAL_PTR(exp, real) ASSERT_EQUAL_U(((uintptr_t)(exp)), ((uintptr_t)(real)))
#define ASSERT_NOT_EQUAL_PTR(exp, real) ASSERT_NOT_EQUAL_U(((uintptr_t)(exp)), ((uintptr_t)(real)))

#endif // _CTEST_PLUS_H_
