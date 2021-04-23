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

#ifndef _XPC_PRIVATE_ENDPOINT_H_
#define _XPC_PRIVATE_ENDPOINT_H_

#include <xpc/xpc.h>
#include <xpc/endpoint.h>

__BEGIN_DECLS

int xpc_endpoint_compare(xpc_endpoint_t xlhs, xpc_endpoint_t xrhs);
mach_port_t xpc_endpoint_copy_listener_port_4sim(xpc_endpoint_t xendpoint);
xpc_endpoint_t xpc_endpoint_create_bs_named(const char* name, uint64_t flags, uint8_t* out_type);
xpc_endpoint_t xpc_endpoint_create_mach_port_4sim(mach_port_t port);

__END_DECLS

#endif //_XPC_PRIVATE_ENDPOINT_H_
