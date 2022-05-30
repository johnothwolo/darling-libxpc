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

#ifndef _XPC_SERIALIZATION_H_
#define _XPC_SERIALIZATION_H_

#include <stdint.h>
#include <stddef.h>
#include <mach/mach.h>
#include <xpc/xpc.h>

#ifdef __cplusplus
extern "C" {
#endif

// our implementation of {de,}serialization tries to replicate Apple's over-the-wire format and is inspired by/based off:
// - http://blog.wuntee.sexy/CVE-2015-3795
// - https://github.com/saelo/pwn2own2018/blob/master/libspc/serialization.c

// notes on the format:
// - it seems that all numbers are always transmitted in little-endian format.
// - sizes do not include the size of the structure containing them, only the content.

//
// structures/values describing the over-the-wire format
//

// "@XPC"
#define XPC_SERIAL_MAGIC 0x40585043

#define XPC_SERIAL_CURRENT_VERSION 5

#define XPC_SERIAL_TYPE_NULL             0x01000
#define XPC_SERIAL_TYPE_BOOL             0x02000
#define XPC_SERIAL_TYPE_INT64            0x03000
#define XPC_SERIAL_TYPE_UINT64           0x04000
#define XPC_SERIAL_TYPE_DOUBLE           0x05000
#define XPC_SERIAL_TYPE_POINTER          0x06000
#define XPC_SERIAL_TYPE_DATE             0x07000
#define XPC_SERIAL_TYPE_DATA             0x08000
#define XPC_SERIAL_TYPE_STRING           0x09000
#define XPC_SERIAL_TYPE_UUID             0x0a000
#define XPC_SERIAL_TYPE_FD               0x0b000
#define XPC_SERIAL_TYPE_SHMEM            0x0c000
#define XPC_SERIAL_TYPE_MACH_SEND        0x0d000
#define XPC_SERIAL_TYPE_ARRAY            0x0e000
#define XPC_SERIAL_TYPE_DICT             0x0f000
#define XPC_SERIAL_TYPE_ERROR            0x10000
#define XPC_SERIAL_TYPE_CONNECTION       0x11000
#define XPC_SERIAL_TYPE_ENDPOINT         0x12000
#define XPC_SERIAL_TYPE_SERIALIZER       0x13000
#define XPC_SERIAL_TYPE_PIPE             0x14000
#define XPC_SERIAL_TYPE_MACH_RECV        0x15000
#define XPC_SERIAL_TYPE_BUNDLE           0x16000
#define XPC_SERIAL_TYPE_SERVICE          0x17000
#define XPC_SERIAL_TYPE_SERVICE_INSTANCE 0x18000
#define XPC_SERIAL_TYPE_ACTIVITY         0x19000
#define XPC_SERIAL_TYPE_FILE_TRANSFER    0x1a000

#define XPC_SERIAL_TYPE_INVALID 0
#define XPC_SERIAL_TYPE_MIN     XPC_SERIAL_TYPE_NULL
#define XPC_SERIAL_TYPE_MAX     XPC_SERIAL_TYPE_FILE_TRANSFER

// "w00t"
#define XPC_MSGH_ID_CHECKIN      0x77303074
#define XPC_MSGH_ID_MESSAGE      0x10000000
#define XPC_MSGH_ID_ASYNC_REPLY  0x20000000
#define XPC_MSGH_ID_NOTIFICATION 0x30000000
// NOTE: i'm unsure as to exact the purpose of this msgh_id, but this is a good guess
#define XPC_MSGH_ID_SYNC_MESSAGE 0x40000000

typedef uint32_t xpc_serial_type_t;
typedef uint32_t xpc_serial_message_id_t;

// common to all XPC object serial representations
typedef struct XPC_PACKED xpc_serial_base {
	xpc_serial_type_t type;
} xpc_serial_base_t;

// for types like dictionary and array.
typedef struct XPC_PACKED xpc_serial_container {
	xpc_serial_base_t base;
	uint32_t size;
	uint32_t entry_count;
	char content[];
} xpc_serial_container_t;

// vld = variable-length data (i.e. strings and data arrays).
typedef struct XPC_PACKED xpc_serial_vld {
	xpc_serial_base_t base;
	uint32_t size;
	char content[];
} xpc_serial_vld_t;

// integral data like integers, booleans, and UUIDs.
typedef struct XPC_PACKED xpc_serial_integral {
	xpc_serial_base_t base;
	char content[];
} xpc_serial_integral_t;

// types that only need to be present; they don't actually carray any value.
// examples: fds, ports, and null.
typedef struct XPC_PACKED xpc_serial_no_content {
	xpc_serial_base_t base;
} xpc_serial_no_content_t;

// header for a single XPC message.
typedef struct XPC_PACKED xpc_serial_header {
	uint32_t magic;
	uint32_t version;
	xpc_serial_base_t contents[];
} xpc_serial_header_t;

// message format used for the checkin message.
typedef struct XPC_PACKED xpc_checkin_message {
	mach_msg_header_t header;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t server_receive_port;
	mach_msg_port_descriptor_t server_send_port;
} xpc_checkin_message_t;

XPC_INLINE
uint64_t xpc_serial_padded_length(uint64_t length) {
	return (length + 3) & -4;
};

#ifdef __cplusplus
};
#endif

#if __OBJC__
	#import <xpc/objects/serializer.h>
	#import <xpc/objects/deserializer.h>
#endif

#endif // _XPC_SERIALIZATION_H_
