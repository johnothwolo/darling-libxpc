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

#import <xpc/objects/service_instance.h>
#import <xpc/xpc.h>
#import <xpc/endpoint.h>
#import <xpc/util.h>

XPC_CLASS_SYMBOL_DECL(service_instance);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(service_instance)

XPC_CLASS_HEADER(service_instance);

@end

//
// private C API
//

XPC_EXPORT
void xpc_service_instance_dup2(xpc_service_instance_t xinstance, int fd, int new_fd) {
	xpc_stub();
};

XPC_EXPORT
void* xpc_service_instance_get_context(xpc_service_instance_t xinstance) {
	// unsure about the return type, but given the name, it's the most likely type
	xpc_stub();
	return NULL;
};

XPC_EXPORT
pid_t xpc_service_instance_get_host_pid(xpc_service_instance_t xinstance) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
pid_t xpc_service_instance_get_pid(xpc_service_instance_t xinstance) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int xpc_service_instance_get_type(xpc_service_instance_t xinstance) {
	// unsure about the return type
	xpc_stub();
	return -1;
};

XPC_EXPORT
bool xpc_service_instance_is_configurable(xpc_service_instance_t xinstance) {
	xpc_stub();
	return false;
};

XPC_EXPORT
void xpc_service_instance_run(xpc_service_instance_t xinstance) {
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_binpref(xpc_service_instance_t xinstance, int binpref) {
	// unsure about `binpref`'s type
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_context(xpc_service_instance_t xinstance, void* context) {
	// unsure about `context`'s type, but given the name, it's the most likely type
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_cwd(xpc_service_instance_t xinstance, const char* cwd) {
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_endpoint(xpc_service_instance_t xinstance, xpc_endpoint_t endpoint) {
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_environment(xpc_service_instance_t xinstance, xpc_object_t environment) {
	// i'm pretty sure `environment` is a dictionary
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_finalizer_f(xpc_service_instance_t xinstance, void (*finalizer)(void* context)) {
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_jetsam_properties(xpc_service_instance_t xinstance, int flags, int priority, int memlimit) {
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_path(xpc_service_instance_t xinstance, const char* path) {
	xpc_stub();
};

XPC_EXPORT
void xpc_service_instance_set_start_suspended(xpc_service_instance_t xinstance) {
	// takes no other arguments; supposed to set the "start suspended" internal flag
	xpc_stub();
};
