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
	xpc_stub();
	return NULL;
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
