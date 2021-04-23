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

#ifndef _XPC_OBJECTS_PLIST_BINARY_V0_DESERIALIZER_H_
#define _XPC_OBJECTS_PLIST_BINARY_V0_DESERIALIZER_H_

#import <xpc/objects/base.h>

#include <stdint.h>

// NOTE: this class is not present in the official libxpc

// we have this class to help with binary v0 plist parsing

XPC_CLASS_DECL(plist_binary_v0_deserializer);

struct xpc_plist_binary_v0_deserializer_s {
	struct xpc_object_s base;
	const uint8_t* data;
	NSUInteger length;
	size_t offset_size;
	size_t reference_size;
	uint64_t object_count;
	uint64_t root_object_reference_number;
	uint64_t offset_table_offset;
	const uint8_t* offset_table;
	XPC_CLASS(object)* root_object;
};

@class XPC_CLASS(int64);
@class XPC_CLASS(double);
@class XPC_CLASS(date);
@class XPC_CLASS(data);
@class XPC_CLASS(string);
@class XPC_CLASS(uuid);
@class XPC_CLASS(array);
@class XPC_CLASS(dictionary);

@interface XPC_CLASS_INTERFACE(plist_binary_v0_deserializer)

@property(readonly) XPC_CLASS(object)* rootObject;

- (instancetype)initWithData: (const void*)data length: (NSUInteger)length;

- (XPC_CLASS(object)*)readObject: (NSUInteger)referenceNumber;

// helper methods

- (NSUInteger)readOffset: (NSUInteger)referenceNumber;
- (NSUInteger)readLength: (const uint8_t*)object dataStart: (const uint8_t**)dataStart;
- (NSUInteger)readReferenceNumber: (const uint8_t*)start next: (const uint8_t**)next;

- (XPC_CLASS(int64)*)readInteger: (const uint8_t*)object;
- (XPC_CLASS(double)*)readReal: (const uint8_t*)object;
- (XPC_CLASS(date)*)readDate: (const uint8_t*)object;
- (XPC_CLASS(data)*)readData: (const uint8_t*)object;
- (XPC_CLASS(string)*)readASCIIString: (const uint8_t*)object;
- (XPC_CLASS(string)*)readUTF16String: (const uint8_t*)object;
- (XPC_CLASS(uuid)*)readUUID: (const uint8_t*)object;
- (XPC_CLASS(array)*)readArray: (const uint8_t*)object;
- (XPC_CLASS(array)*)readSet: (const uint8_t*)object;
- (XPC_CLASS(dictionary)*)readDictionary: (const uint8_t*)object;

@end

#endif // _XPC_OBJECTS_PLIST_BINARY_V0_DESERIALIZER_H_
