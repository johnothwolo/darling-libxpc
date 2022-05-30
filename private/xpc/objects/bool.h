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

#ifndef _XPC_OBJECTS_BOOL_H_
#define _XPC_OBJECTS_BOOL_H_

#import <xpc/objects/base.h>

// bools are NOT simple wrapper objects because they are global singletons

XPC_CLASS_DECL(bool);

struct xpc_bool_s {
	struct xpc_object_s base;
	bool value;
};

#undef bool
@interface XPC_CLASS_INTERFACE(bool)

@property(assign) BOOL value;

- (instancetype)initWithValue: (BOOL)value;
+ (instancetype)boolForValue: (BOOL)value;

@end
#define bool _Bool

#endif // _XPC_OBJECTS_BOOL_H_
