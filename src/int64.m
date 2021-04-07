#import <xpc/objects/int64.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/serialization.h>

XPC_WRAPPER_CLASS_IMPL(int64, int64_t, "%lld");
XPC_WRAPPER_CLASS_SERIAL_IMPL(int64, int64_t, INT64, U64, uint64_t);

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_int64_create(int64_t value) {
	return [[XPC_CLASS(int64) alloc] initWithValue: value];
};

XPC_EXPORT
int64_t xpc_int64_get_value(xpc_object_t xint) {
	TO_OBJC_CHECKED(int64, xint, integer) {
		return integer.value;
	}
	return 0;
};

//
// private C API
//

XPC_EXPORT
void _xpc_int64_set_value(xpc_object_t xint, int64_t value) {
	TO_OBJC_CHECKED(int64, xint, integer) {
		integer.value = value;
	}
};
