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

#ifndef _XPC_OBJECTS_SERVICE_INSTANCE_H_
#define _XPC_OBJECTS_SERVICE_INSTANCE_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(service_instance);

struct xpc_service_instance_s {
	struct xpc_object_s base;
};

@interface XPC_CLASS_INTERFACE(service_instance)

@end

#endif // _XPC_OBJECTS_SERVICE_INSTANCE_H_
