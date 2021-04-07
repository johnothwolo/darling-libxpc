#import <xpc/objects/date.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <Foundation/NSDate.h>
#import <xpc/serialization.h>

#include <time.h>

XPC_WRAPPER_CLASS_IMPL(date, int64_t, "%lld");
XPC_WRAPPER_CLASS_SERIAL_IMPL(date, int64_t, DATE, U64, uint64_t);

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_date_create(int64_t value) {
	return [[XPC_CLASS(date) alloc] initWithValue: value];
};

XPC_EXPORT
xpc_object_t xpc_date_create_from_current(void) {
	return xpc_date_create((int64_t)clock_gettime_nsec_np(CLOCK_REALTIME));
};

XPC_EXPORT
int64_t xpc_date_get_value(xpc_object_t xdate) {
	TO_OBJC_CHECKED(date, xdate, date) {
		return date.value;
	}
	return 0;
};

//
// private C API
//

XPC_EXPORT
xpc_object_t xpc_date_create_absolute(int64_t value) {
	return [[XPC_CLASS(date) alloc] initWithValue: (value + NSTimeIntervalSince1970) * NSEC_PER_SEC];
};

XPC_EXPORT
int64_t xpc_date_get_value_absolute(xpc_object_t xdate) {
	TO_OBJC_CHECKED(date, xdate, date) {
		return (date.value / NSEC_PER_SEC) - NSTimeIntervalSince1970;
	}
	return 0;
};

XPC_EXPORT
bool xpc_date_is_int64_range(xpc_object_t xdate) {
	TO_OBJC_CHECKED(date, xdate, date) {
		return true;
	}
	return false;
};
