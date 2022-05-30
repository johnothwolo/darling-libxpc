/**
 * This file is part of Darling.
 *
 * Copyright (C) 2021 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

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
	xpc_object_t obj = NULL;
	// size_t alloced = 0, ret;
  	// mach_port_t object_handle;
	
	// if (malloc_size((const void *)region))
    // 	xpc_abort("Attempt to pass a malloc(3)ed region to xpc_shmem_create().");
	// ret = mach_make_memory_entry_64(mach_task_self(), &alloced, region, VM_PROT_READ | VM_PROT_WRITE | MAP_MEM_VM_SHARE | VM_PROT_IS_MASK, &object_handle, '\0');
	xpc_stub();
	return obj;
};

XPC_EXPORT
size_t xpc_shmem_map(xpc_object_t xshmem, void** region) {
	xpc_stub();
	return 0;
};

//
// private C API
//

XPC_EXPORT
mach_port_t _xpc_shmem_get_mach_port(xpc_object_t xshmem) {
	xpc_stub();
	return MACH_PORT_NULL;
};

XPC_EXPORT
xpc_object_t xpc_shmem_create_readonly(const void* region, size_t length) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
size_t xpc_shmem_get_length(xpc_object_t xshmem) {
	xpc_stub();
	return 0;
};
