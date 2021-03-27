#import <xpc/objects/shmem.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(shmem);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(shmem)

XPC_CLASS_HEADER(shmem);

@end

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_shmem_create(void* region, size_t length) {
	return NULL;
};

XPC_EXPORT
size_t xpc_shmem_map(xpc_object_t xshmem, void** region) {
	return 0;
};

//
// private C API
//

XPC_EXPORT
mach_port_t _xpc_shmem_get_mach_port(xpc_object_t xshmem) {
	return MACH_PORT_NULL;
};

XPC_EXPORT
xpc_object_t xpc_shmem_create_readonly(const void* region, size_t length) {
	return NULL;
};

XPC_EXPORT
size_t xpc_shmem_get_length(xpc_object_t xshmem) {
	return 0;
};
