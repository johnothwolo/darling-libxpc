#ifndef _CTEST_PLUS_H_
#define _CTEST_PLUS_H_

#include <ctest.h>

// convenience header that adds additional macros for ctest

#define ASSERT_EQUAL_PTR(exp, real) ASSERT_EQUAL_U(((uintptr_t)(exp)), ((uintptr_t)(real)))
#define ASSERT_NOT_EQUAL_PTR(exp, real) ASSERT_NOT_EQUAL_U(((uintptr_t)(exp)), ((uintptr_t)(real)))

#endif // _CTEST_PLUS_H_
