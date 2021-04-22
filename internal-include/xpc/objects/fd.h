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

#ifndef _XPC_OBJECTS_FD_H_
#define _XPC_OBJECTS_FD_H_

#import <xpc/objects/base.h>

#include <mach/mach_port.h>

XPC_CLASS_DECL(fd);

struct xpc_fd_s {
	struct xpc_object_s base;
	mach_port_t port;
};

@interface XPC_CLASS_INTERFACE(fd)

@property(readonly) mach_port_t port;

- (instancetype)initWithDescriptor: (int)descriptor;
- (instancetype)initWithPort: (mach_port_t)port;

- (int)instantiateDescriptor;

@end

#endif // _XPC_OBJECTS_FD_H_
