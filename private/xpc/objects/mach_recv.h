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

#ifndef _XPC_OBJECTS_MACH_RECV_H_
#define _XPC_OBJECTS_MACH_RECV_H_

#import <xpc/objects/base.h>
#import <mach/port.h>

XPC_IGNORE_DUPLICATE_PROTOCOL_PUSH;
XPC_CLASS_DECL(mach_recv);
XPC_IGNORE_DUPLICATE_PROTOCOL_POP;

struct xpc_mach_recv_s {
	struct xpc_object_s base;
	mach_port_t port;
};

@interface XPC_CLASS_INTERFACE(mach_recv)

@property(readonly) mach_port_t port;

- (instancetype)initWithPort: (mach_port_t)port;

- (mach_port_t)extractPort;

@end

//
// private C API
//

xpc_object_t xpc_mach_recv_create(mach_port_t recv);
mach_port_t xpc_mach_recv_extract_right(xpc_object_t xrecv);

#endif // _XPC_OBJECTS_MACH_RECV_H_
