#ifndef _XPC_PRIVATE_BUNDLE_H_
#define _XPC_PRIVATE_BUNDLE_H_

#include <xpc/xpc.h>

__BEGIN_DECLS

OS_ENUM(xpc_bundle_error, int,
	xpc_bundle_error_success,
	xpc_bundle_error_failed_to_read_plist = -1,
	xpc_bundle_error_invalid_plist = -2,
);

xpc_object_t xpc_bundle_create(const char* path, unsigned int flags);
xpc_object_t xpc_bundle_create_from_origin(unsigned int origin, const char* path);
xpc_object_t xpc_bundle_create_main(void);

xpc_object_t xpc_bundle_copy_info_dictionary(xpc_object_t xbundle);
char* xpc_bundle_copy_resource_path(xpc_object_t xbundle, const char* name, const char* type);
xpc_object_t xpc_bundle_copy_services(xpc_object_t xbundle);

int xpc_bundle_get_error(xpc_object_t xbundle);
const char* xpc_bundle_get_executable_path(xpc_object_t xbundle);
xpc_object_t xpc_bundle_get_info_dictionary(xpc_object_t xbundle);
const char* xpc_bundle_get_path(xpc_object_t xbundle);
uint64_t xpc_bundle_get_property(xpc_object_t xbundle, unsigned int property);
xpc_object_t xpc_bundle_get_xpcservice_dictionary(xpc_object_t xbundle);

typedef void (*xpc_bundle_resolution_callback_f)(xpc_object_t bundle, int error, void* context);

void xpc_bundle_populate(xpc_object_t xbundle, xpc_object_t info_dictionary, xpc_object_t services_array);
void xpc_bundle_resolve(xpc_object_t xbundle, dispatch_queue_t queue_for_later, void* context, xpc_bundle_resolution_callback_f callback);
void xpc_bundle_resolve_on_queue(xpc_object_t xbundle, dispatch_queue_t queue_for_later, dispatch_queue_t queue_for_now, void* context, xpc_bundle_resolution_callback_f callback);
void xpc_bundle_resolve_sync(xpc_object_t xbundle);

void xpc_add_bundle(const char* path, unsigned int flags);
void xpc_add_bundles_for_domain(xpc_object_t domain, xpc_object_t bundles);

__END_DECLS

#endif // _XPC_PRIVATE_BUNDLE_H_
