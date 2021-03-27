#include "ctest-plus.h"
#include <xpc/private.h>
#include "test-util.h"

CTEST_DATA(array) {
	xpc_object_t objects[OBJECT_COUNT];
	xpc_object_t array;
};

CTEST_SETUP(array) {
	create_objects(data->objects, OBJECT_COUNT);
	data->array = xpc_array_create(data->objects, OBJECT_COUNT);
};

CTEST_TEARDOWN(array) {
	xpc_release(data->array);
	release_objects(data->objects, OBJECT_COUNT);
};

CTEST(array, create_empty) {
	xpc_object_t array = xpc_array_create(NULL, 0);
	ASSERT_NOT_NULL(array);
	ASSERT_EQUAL_U(0, xpc_array_get_count(array));
	xpc_release(array);
};

CTEST(array, create_nonempty) {
	xpc_object_t objects[OBJECT_COUNT];
	create_objects(objects, OBJECT_COUNT);
	xpc_object_t array = xpc_array_create(objects, OBJECT_COUNT);

	ASSERT_NOT_NULL(array);
	ASSERT_EQUAL_U(OBJECT_COUNT, xpc_array_get_count(array));
	for (size_t i = 0; i < OBJECT_COUNT; ++i) {
		// we have a reference but the array should have one too
		NSUInteger retainCount = [objects[i] retainCount];
		// global objects have NSUIntegerMax as their reference count
		if (retainCount != NSUIntegerMax) {
			ASSERT_EQUAL_U(2, retainCount);
		}
	}

	xpc_release(array);

	for (size_t i = 0; i < OBJECT_COUNT; ++i) {
		// we should have the only reference now
		NSUInteger retainCount = [objects[i] retainCount];
		if (retainCount != NSUIntegerMax) {
			ASSERT_EQUAL_U(1, retainCount);
		}
	}

	release_objects(objects, OBJECT_COUNT);
};

CTEST2(array, get_value) {
	size_t index = rand_index(OBJECT_COUNT);

	ASSERT_EQUAL_PTR(data->objects[index], xpc_array_get_value(data->array, index));
};

CTEST2(array, set_value) {
	xpc_object_t newObject = xpc_int64_create(10);
	NSUInteger oldRefCount = 0;
	size_t index = rand_index(OBJECT_COUNT);

	xpc_array_set_value(data->array, index, newObject);
	// setting the new value at that index should have dropped the old value
	oldRefCount = [data->objects[index] retainCount];
	if (oldRefCount != NSUIntegerMax) {
		ASSERT_EQUAL_U(1, [data->objects[index] retainCount]);
	}
	// and it should have assigned and retained the new value
	ASSERT_EQUAL_PTR(newObject, xpc_array_get_value(data->array, index));
	ASSERT_EQUAL_U(2, [newObject retainCount]);

	xpc_release(newObject);
};

CTEST2(array, append_value) {
	xpc_object_t newObject = xpc_int64_create(10);
	xpc_object_t newObject2 = xpc_int64_create(15);

	xpc_array_append_value(data->array, newObject);
	ASSERT_EQUAL_U(OBJECT_COUNT + 1, xpc_array_get_count(data->array));
	ASSERT_EQUAL_PTR(newObject, xpc_array_get_value(data->array, OBJECT_COUNT));

	xpc_array_set_value(data->array, XPC_ARRAY_APPEND, newObject2);
	ASSERT_EQUAL_U(OBJECT_COUNT + 2, xpc_array_get_count(data->array));
	ASSERT_EQUAL_PTR(newObject2, xpc_array_get_value(data->array, OBJECT_COUNT + 1));

	xpc_release(newObject2);
	xpc_release(newObject);
};

static void visitor(size_t index, xpc_object_t value, void* context) {
	size_t* visitedCount = context;
	++*visitedCount;
};

CTEST2(array, apply) {
	__block size_t visitedCount = 0;

	xpc_array_apply(data->array, ^bool(size_t index, xpc_object_t value) {
		++visitedCount;
		return true;
	});
	ASSERT_EQUAL_U(OBJECT_COUNT, visitedCount);

	visitedCount = 0;
	xpc_array_apply(data->array, ^bool(size_t index, xpc_object_t value) {
		++visitedCount;
		if (index == OBJECT_COUNT / 2) {
			return false;
		}
		return true;
	});
	ASSERT_EQUAL_U((OBJECT_COUNT / 2) + 1, visitedCount);

	visitedCount = 0;
	xpc_array_apply_f(data->array, &visitedCount, visitor);
	ASSERT_EQUAL_U(OBJECT_COUNT, visitedCount);
};

// there are too many getters and setters and i don't want to write tests for all of them
// (and doing it with a macro would be tricky for some of them like fd or data)
//
// if the basic API works, everything should
// (all the getters and setters use the basic API to do their stuff)
