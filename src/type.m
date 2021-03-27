#import <xpc/xpc.h>
#import <xpc/util.h>
#import <objc/runtime.h>

//
// C API
//

XPC_EXPORT
xpc_type_t xpc_get_type(xpc_object_t xobject) {
	TO_OBJC_CHECKED(object, xobject, object) {
		return (xpc_type_t)[object class];
	}
	return NULL;
};



//
// private C API
//

XPC_EXPORT
const char* xpc_type_get_name(xpc_type_t xtype) {
	return class_getName((Class)xtype);
};

XPC_EXPORT
Class xpc_get_class4NSXPC(xpc_type_t xtype) {
	return (Class)xtype;
};
XPC_EXPORT
bool xpc_is_kind_of_xpc_object4NSXPC(xpc_object_t object) {
	return [object isKindOfClass: [XPC_CLASS(object) class]];
};

struct some_extension_type_struct;

XPC_EXPORT
void xpc_extension_type_init(struct some_extension_type_struct* extension_type, void* some_user_function_probably) {

};
