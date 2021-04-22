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

#ifndef _XPC_OBJECTS_ARRAY_H_
#define _XPC_OBJECTS_ARRAY_H_

#import <xpc/objects/base.h>
#import <Foundation/NSEnumerator.h>

XPC_CLASS_DECL(array);

struct xpc_array_s {
	struct xpc_object_s base;
	unsigned long size; // not NSUInteger or size_t because it needs to be `unsigned long` in 32-bit builds as well
	XPC_CLASS(object)** array;
};

@interface XPC_CLASS_INTERFACE(array)

// this API is modeled after NSMutableArray

@property(readonly) NSUInteger count;

- (instancetype)initWithObjects: (XPC_CLASS(object)* const*)objects count: (NSUInteger)count;

- (XPC_CLASS(object)*)objectAtIndex: (NSUInteger)index;
- (void)addObject: (XPC_CLASS(object)*)object;
- (void)replaceObjectAtIndex: (NSUInteger)index withObject: (XPC_CLASS(object)*)object;
- (void)enumerateObjectsUsingBlock: (void (^)(XPC_CLASS(object)* object, NSUInteger index, BOOL* stop))block;
- (XPC_CLASS(object)*)objectAtIndexedSubscript: (NSUInteger)index;
- (void)setObject: (XPC_CLASS(object)*)object atIndexedSubscript: (NSUInteger)index;
- (NSUInteger)countByEnumeratingWithState: (NSFastEnumerationState*)state objects: (id __unsafe_unretained [])objects count: (NSUInteger)count;

@end

#endif // _XPC_OBJECTS_ARRAY_H_
