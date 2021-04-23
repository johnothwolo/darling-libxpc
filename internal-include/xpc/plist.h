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

#ifndef _XPC_INTERNAL_PLIST_H_
#define _XPC_INTERNAL_PLIST_H_

#import <xpc/objects/plist_xml_element.h>
#import <xpc/objects/plist_binary_v0_deserializer.h>

#include <stdint.h>

// NOTE: multibyte values are big-endian!
typedef struct XPC_PACKED xpc_plist_binary_v0_trailer {
	uint8_t unused[6];
	uint8_t offset_size;
	uint8_t reference_size;
	uint64_t object_count;
	uint64_t root_object_reference_number;
	uint64_t offset_table_offset;
} xpc_plist_binary_v0_trailer_t;

OS_ENUM(xpc_plist_binary_v0_object_type, uint8_t,
	// combination of null, false, and true (as well as useless fillers)
	xpc_plist_binary_v0_object_type_singleton    = 0,
	xpc_plist_binary_v0_object_type_integer      = 1,
	xpc_plist_binary_v0_object_type_real         = 2,
	xpc_plist_binary_v0_object_type_date         = 3,
	xpc_plist_binary_v0_object_type_data         = 4,
	xpc_plist_binary_v0_object_type_ascii_string = 5,
	xpc_plist_binary_v0_object_type_utf16_string = 6,
	xpc_plist_binary_v0_object_type_uuid         = 8,
	xpc_plist_binary_v0_object_type_array        = 10,
	xpc_plist_binary_v0_object_type_set          = 12,
	xpc_plist_binary_v0_object_type_dictionary   = 13,
);

#endif // _XPC_INTERNAL_PLIST_H_
