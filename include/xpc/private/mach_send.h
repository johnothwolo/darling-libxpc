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

#ifndef _XPC_PRIVATE_MACH_SEND_H_
#define _XPC_PRIVATE_MACH_SEND_H_

#include <xpc/xpc.h>
#include <mach/mach.h>

__BEGIN_DECLS

XPC_EXPORT
XPC_TYPE(_xpc_type_mach_send);
#define XPC_TYPE_MACH_SEND (&_xpc_type_mach_send)

XPC_IGNORE_DUPLICATE_PROTOCOL_PUSH;
XPC_DECL(xpc_mach_send);
XPC_IGNORE_DUPLICATE_PROTOCOL_POP;

xpc_object_t xpc_mach_send_create(mach_port_t send);
xpc_object_t xpc_mach_send_create_with_disposition(mach_port_t send, unsigned int disposition);
mach_port_t xpc_mach_send_copy_right(xpc_object_t xsend);

mach_port_t xpc_mach_send_get_right(xpc_object_t xsend);
mach_port_t xpc_mach_send_extract_right(xpc_object_t xsend);

__END_DECLS

#endif // _XPC_PRIVATE_MACH_SEND_H_
