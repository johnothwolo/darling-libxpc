#import <xpc/objects/double.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

#include <math.h>

XPC_WRAPPER_CLASS_IMPL(double, double, "%f");

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_double_create(double value) {
	return [[XPC_CLASS(double) alloc] initWithValue: value];
};

XPC_EXPORT
double xpc_double_get_value(xpc_object_t xdouble) {
	TO_OBJC_CHECKED(double, xdouble, doubleObj) {
		return doubleObj.value;
	}
	return NAN;
};

//
// private C API
//

XPC_EXPORT
void _xpc_double_set_value(xpc_object_t xdouble, double value) {
	TO_OBJC_CHECKED(double, xdouble, doubleObj) {
		doubleObj.value = value;
	}
};
