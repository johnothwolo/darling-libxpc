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

#ifndef _XPC_PRIVATE_PLIST_H_
#define _XPC_PRIVATE_PLIST_H_

#include <xpc/xpc.h>

__BEGIN_DECLS

xpc_object_t xpc_create_from_plist(const void* _data, size_t length);
void xpc_create_from_plist_descriptor(int fd, dispatch_queue_t queue, void(^callback)(xpc_object_t result));

__END_DECLS

#endif // _XPC_PRIVATE_PLIST_H_
