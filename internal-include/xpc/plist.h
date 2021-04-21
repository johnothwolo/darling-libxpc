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
