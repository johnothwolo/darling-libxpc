#import <xpc/objects/pointer.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

XPC_WRAPPER_CLASS_IMPL(pointer, void*, "%p");

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_pointer_create(void* value) {
	return [[XPC_CLASS(pointer) alloc] initWithValue: value];
};

XPC_EXPORT
void* xpc_pointer_get_value(xpc_object_t xptr) {
	TO_OBJC_CHECKED(pointer, xptr, ptr) {
		return ptr.value;
	}
	return NULL;
};
