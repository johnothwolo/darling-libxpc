#import <xpc/objects/uint64.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

XPC_WRAPPER_CLASS_IMPL(uint64, uint64_t, "%llu");

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_uint64_create(uint64_t value) {
	return [[XPC_CLASS(uint64) alloc] initWithValue: value];
};

XPC_EXPORT
uint64_t xpc_uint64_get_value(xpc_object_t xuint) {
	TO_OBJC_CHECKED(uint64, xuint, uinteger) {
		return uinteger.value;
	}
	return 0;
};
