#import <xpc/objects/bundle.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(bundle);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(bundle)

XPC_CLASS_HEADER(bundle);

@end

//
// private C API
//

XPC_EXPORT
xpc_object_t xpc_bundle_copy_info_dictionary(xpc_bundle_t xbundle) {
	// doesn't actually copy the dictionary; only retains it
	xpc_stub();
	return NULL;
};

XPC_EXPORT
char* xpc_bundle_copy_resource_path(xpc_bundle_t xbundle) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_bundle_copy_services(xpc_bundle_t xbundle) {
	// doesn't actually copy the array; only retains it
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_bundle_t xpc_bundle_create(const char* path, unsigned int flags) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_bundle_t xpc_bundle_create_from_origin(unsigned int origin, const char* path) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_bundle_t xpc_bundle_create_main(void) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
int xpc_bundle_get_error(xpc_bundle_t xbundle) {
	// unsure about the return type
	xpc_stub();
	return -1;
};

XPC_EXPORT
const char* xpc_bundle_get_executable_path(xpc_bundle_t xbundle) {
	// does NOT copy the path it returns
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_bundle_get_info_dictionary(xpc_bundle_t xbundle) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
const char* xpc_bundle_get_path(xpc_bundle_t xbundle) {
	// does NOT copy the path it returns
	xpc_stub();
	return NULL;
};

XPC_EXPORT
uint64_t xpc_bundle_get_property(xpc_bundle_t xbundle, unsigned int property) {
	// unsure about the return type; it can actually return both integers and pointers
	xpc_stub();
	return 0;
};

XPC_EXPORT
xpc_object_t xpc_bundle_get_xpcservice_dictionary(xpc_bundle_t xbundle) {
	// the dictionary is contained within the info dictionary
	xpc_stub();
	return NULL;
};

XPC_EXPORT
void xpc_bundle_populate(xpc_bundle_t xbundle, xpc_object_t info_dictionary, xpc_object_t services_array) {
	xpc_stub();
};

XPC_EXPORT
void xpc_bundle_resolve(xpc_bundle_t xbundle, dispatch_queue_t queue_for_later, void* something, void* something_else) {
	// no clue what `something` and `something_else` are
	xpc_stub();
};

XPC_EXPORT
void xpc_bundle_resolve_on_queue(xpc_bundle_t xbundle, dispatch_queue_t queue_for_later, dispatch_queue_t queue_for_now, void* something, void* something_else) {
	// no clue what `something` and `something_else` are
	xpc_stub();
};

XPC_EXPORT
void xpc_bundle_resolve_sync(xpc_bundle_t xbundle) {
	xpc_stub();
};

XPC_EXPORT
void xpc_add_bundle(const char* path, unsigned int flags) {
	xpc_stub();
};

XPC_EXPORT
void xpc_add_bundles_for_domain(xpc_object_t domain, xpc_object_t bundles) {
	xpc_stub();
};
