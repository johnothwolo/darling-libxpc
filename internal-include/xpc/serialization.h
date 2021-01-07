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

// "XPC!"
#define XPC_SERIAL_MAGIC 0x58504321

#define XPC_SERIAL_CURRENT_VERSION 5

#define XPC_SERIAL_TYPE_NULL      0x1000
#define XPC_SERIAL_TYPE_BOOL      0x2000
#define XPC_SERIAL_TYPE_INT64     0x3000
#define XPC_SERIAL_TYPE_UINT64    0x4000
#define XPC_SERIAL_TYPE_DOUBLE    0x5000
#define XPC_SERIAL_TYPE_DATA      0x8000
#define XPC_SERIAL_TYPE_STRING    0x9000
#define XPC_SERIAL_TYPE_UUID      0xa000
#define XPC_SERIAL_TYPE_FD        0xb000
#define XPC_SERIAL_TYPE_SHMEM     0xc000
#define XPC_SERIAL_TYPE_SEND_PORT 0xd000
#define XPC_SERIAL_TYPE_ARRAY     0xe000
#define XPC_SERIAL_TYPE_DICT      0xf000
#define XPC_SERIAL_TYPE_RECV_PORT 0x15000

#define XPC_SERIAL_TYPE_INVALID 0
#define XPC_SERIAL_TYPE_MIN     XPC_SERIAL_TYPE_NULL
#define XPC_SERIAL_TYPE_MAX     XPC_SERIAL_TYPE_RECV_PORT

// for types like dictionary and array.
typedef struct __attribute__((packed)) xpc_serial_container {
	uint32_t type;
	uint32_t size;
	uint32_t entry_count;
	char content[];
} xpc_serial_container_t;

// vld = variable-length data (i.e. strings and data arrays).
typedef struct __attribute__((packed)) xpc_serial_vld {
	uint32_t type;
	uint32_t size;
	char content[];
} xpc_serial_vld_t;

// integral data like integers, booleans, and UUIDs.
typedef struct __attribute__((packed)) xpc_serial_integral {
	uint32_t type;
	char content[];
} xpc_serial_integral_t;

// types that only need to be present; they don't actually carray any value.
// examples: fds, ports, and null.
typedef struct __attribute__((packed)) xpc_serial_no_content {
	uint32_t type;
} xpc_serial_no_content_t;

// header for a single XPC message.
typedef struct __attribute__((packed)) xpc_serial_header {
	uint32_t magic;
	uint32_t version;
	xpc_serial_container_t container;
} xpc_serial_header_t;

//
// misc structures for our implementation (irrelevant to the actual serialized format)
//

typedef struct xpc_serial_port_descriptor {
	mach_port_t port;
	mach_msg_type_name_t type;
} xpc_serial_port_descriptor_t;

typedef struct xpc_serializer {
	char* buffer;
	const char* buffer_end;
	char* current_position;
	xpc_serial_port_descriptor_t* ports;
	size_t port_count;
} xpc_serializer_t;

typedef struct xpc_deserializer {
	const char* buffer;
	const char* buffer_end;
	const char* current_position;
} xpc_deserializer_t;

/**
 * Writes the given data into the output buffer.
 *
 * @param serializer The serializer context that we're operating on.
 * @param buffer     Optional. The data to write.
 * @param length     Number of bytes to write from the buffer.
 * @param out_length Optional. Pointer to a value that gets the actual number of bytes the write occupies in the output buffer (including padding) added to it.
 *
 * @returns 0 if it succeeded; negative error code otherwise.
 */
int xpc_serial_write(xpc_serializer_t* serializer, const void* buffer, size_t length, size_t* out_length);

/**
 * Writes the given string into the output buffer.
 *
 * @param serializer The serializer context that we're operating on.
 * @param string     The null-terminated string to write.
 * @param out_length Same as in `xpc_serial_write`.
 */
int xpc_serial_write_string(xpc_serializer_t* serializer, const char* string, size_t* out_length);

/**
 * Writes the given 32-bit unsigned integer into the output buffer.
 *
 * @param serializer The serializer context that we're operating on.
 * @param integer    The 32-bit unsigned integer to write.
 * @param out_length Same as in `xpc_serial_write`.
 */
int xpc_serial_write_u32(xpc_serializer_t* serializer, uint32_t integer, size_t* out_length);

/**
 * Writes the given 64-bit unsigned integer into the output buffer.
 *
 * @param serializer The serializer context that we're operating on.
 * @param integer    The 64-bit unsigned integer to write.
 * @param out_length Same as in `xpc_serial_write`.
 */
int xpc_serial_write_u64(xpc_serializer_t* serializer, uint64_t integer, size_t* out_length);

/**
 * Reserves a chunk of the output buffer for future use.
 *
 * @param serializer          The serializer context that we're operating on.
 * @param reservation_pointer Pointer that will be set to a pointer for the reserved chunk.
 * @param reserved_length     Number of bytes to reserve in the output buffer.
 * @param out_length          Same as in `xpc_serial_write`.
 *
 * @returns 0 if it succeeded; negative error code otherwise.
 *
 * @note It is actually valid for this function to return `NULL` in `reservation_pointer`. This occurs when the serializer context has no buffer associated with it.
 * To determine if the function failed, rely solely on the return code.
 */
int xpc_serial_write_reserve(xpc_serializer_t* serializer, char** reservation_pointer, size_t reserved_length, size_t* out_length);

/**
 * Serializes the given XPC object into the output buffer.
 *
 * @param serializer The serializer context that we're operating on.
 * @param object     The XPC object to serialize.
 * @param out_length Same as in `xpc_serial_write`.
 *
 * @note This function may modify the serializer context and then fail. If you'd like to preserve the serializer state before the call in case of failure, make sure to copy it before calling this function.
 */
int xpc_serial_write_object(xpc_serializer_t* serializer, xpc_object_t object, size_t* out_length);

/**
 * Reads the next available data from the deserialization context.
 *
 * @param deserializer The deserializer context that we're operating on.
 * @param buffer       The buffer to read the data into.
 * @param length       Size in bytes of the given buffer.
 * @param out_length   Optional. Pointer to a value that gets the actual number of bytes the read occupies in the input buffer (including padding) added to it.
 */
int xpc_serial_read(xpc_deserializer_t* deserializer, void* buffer, size_t length, size_t* out_length);

/**
 * Consumes the a chunk of the input buffer without copying it.
 *
 * @param deserializer The deserializer context that we're operating on.
 * @param buffer       Pointer that will be set to a pointer to the start of the chunk that was consumed.
 * @param length       Number of bytes to consume.
 * @param out_length   Same as in `xpc_serial_read`.
 */
int xpc_serial_read_in_place(xpc_deserializer_t* deserializer, const void** buffer, size_t length, size_t* out_length);

/**
 * Returns the current chunk of the input buffer as a string pointer.
 *
 * @param deserializer The deserializer context that we're operating on.
 * @param string       Pointer that will be set to a pointer to the constant string in the input buffer.
 * @param out_length   Same as in `xpc_serial_read`.
 *
 * @note This is **NOT** the exact inverse of `xpc_serial_write_string`, and also does not call `xpc_serial_read` internally.
 * It behaves like `xpc_serial_read_in_place`. It does so to avoid unnecessary string copying (you can do that yourself
 * after calling this method if you really want a copy of the string).
 */
int xpc_serial_read_string(xpc_deserializer_t* deserializer, const char** string, size_t* out_length);

/**
 * Reads a 32-bit unsigned integer from the input buffer.
 *
 * @param deserializer The deserializer context that we're operating on.
 * @param integer      Pointer to a 32-bit unsigned integer where the value will be read into.
 * @param out_length   Same as in `xpc_serial_read`.
 */
int xpc_serial_read_u32(xpc_deserializer_t* deserializer, uint32_t* integer, size_t* out_length);

/**
 * Reads a 64-bit unsigned integer from the input buffer.
 *
 * @param deserializer The deserializer context that we're operating on.
 * @param integer      Pointer to a 64-bit unsigned integer where the value will be read into.
 * @param out_length   Same as in `xpc_serial_read`.
 */
int xpc_serial_read_u64(xpc_deserializer_t* deserializer, uint64_t* integer, size_t* out_length);

/**
 * Deserializes an XPC object from the input buffer.
 *
 * @param deserializer The deserializer context that we're operating on.
 * @param object       Pointer to an XPC object that will be set to the object created from parsing the input buffer.
 * @param out_length   Same as in `xpc_serial_read`.
 *
 * @note This function may modify the deserializer context and then fail. If you'd like to preserve the deserializer state before the call in case of failure, make sure to copy it before calling this function.
 */
int xpc_serial_read_object(xpc_deserializer_t* deserializer, xpc_object_t* object, size_t* out_length);

/**
 * Serializes an XPC object as a message.
 *
 * @param object     The XPC object to serialize.
 * @param buffer     Buffer to output the serialized representation of the XPC object into.
 * @param size       Size of the given buffer in bytes.
 * @param out_length Optional. Pointer to a value that gets the actual number of bytes written to the buffer added to it.
 */
int xpc_serialize(xpc_object_t object, void* buffer, size_t length, size_t* out_length);

/**
 * Deserializes an XPC object from a message.
 *
 * @param buffer     Buffer to read the serialized representation of the XPC object from.
 * @param size       Size of the given buffer in bytes.
 * @param object     Pointer to an XPC object that will be set to the object created from parsing the input buffer.
 * @param out_length Optional. Pointer to a value that gets the actual number of bytes read from the buffer added to it.
 */
int xpc_deserialize(xpc_object_t* object, const void* buffer, size_t length, size_t* out_length);

#ifdef __cplusplus
};
#endif

#endif // _XPC_SERIALIZATION_H_
