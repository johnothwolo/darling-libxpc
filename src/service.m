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

#import <xpc/objects/service.h>
#import <xpc/objects/service_instance.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(service);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(service)

XPC_CLASS_HEADER(service);

@end

//
// private C API
//

XPC_EXPORT
void xpc_service_attach(xpc_service_t xservice, bool run, bool kill) {
	xpc_stub();
};

XPC_EXPORT
xpc_service_t xpc_service_create(int flags, const char* name, uint64_t handle, dispatch_queue_t queue) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_service_t xpc_service_create_from_specifier(const char* specifier, dispatch_queue_t queue) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
const char* xpc_service_get_rendezvous_token(xpc_service_t xservice) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
void xpc_service_kickstart(xpc_service_t xservice, bool suspended, bool kill) {
	xpc_stub();
};

XPC_EXPORT
void xpc_service_set_attach_handler(xpc_service_t xservice, void (^handler)(xpc_service_instance_t xinstance)) {
	xpc_stub();
};

XPC_EXPORT
void _xpc_service_last_xref_cancel(xpc_service_t xservice) {
	xpc_stub();
};

XPC_EXPORT
void xpc_handle_service() {
	// i have no clue what the types for the 3 arguments to this function are
	xpc_stub();
};

XPC_EXPORT
void xpc_handle_subservice() {
	// same as for `xpc_handle_service`
	xpc_stub();
};
