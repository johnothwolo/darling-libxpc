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

#include <xpc/xpc.h>
#include <xpc/internal.h>
#include <xpc/serialization.h>

// works only for multiples of 2
// credit: https://stackoverflow.com/a/9194117/6620880
XPC_INLINE int round_up_multiple_of_2(uint64_t number, uint64_t multiple) {
	return (number + multiple - 1) & -multiple;
};

XPC_INLINE int xpc_serial_space_check(xpc_serializer_t* serializer, size_t space) {
	// no buffer means the user wants to know how much space is necessary to serialize the given object
	// that means we're not actually going to copy anything, so just return success
	if (!serializer->buffer)
		return 0;

	if (serializer->current_position + space > serializer->buffer_end)
		return -1;

	return 0;
};

XPC_INLINE uint32_t xpc_serial_map_type(uint8_t xpc_type) {
	static uint32_t xpc_type_map[] = {
		XPC_SERIAL_TYPE_INVALID, // invalid
		XPC_SERIAL_TYPE_DICT,
		XPC_SERIAL_TYPE_ARRAY,
		XPC_SERIAL_TYPE_BOOL,
		XPC_SERIAL_TYPE_INVALID, // connection
		XPC_SERIAL_TYPE_INVALID, // endpoint
		XPC_SERIAL_TYPE_NULL,
		XPC_SERIAL_TYPE_INVALID, // activity
		XPC_SERIAL_TYPE_INT64,
		XPC_SERIAL_TYPE_UINT64,
		XPC_SERIAL_TYPE_INVALID, // date
		XPC_SERIAL_TYPE_DATA,
		XPC_SERIAL_TYPE_STRING,
		XPC_SERIAL_TYPE_UUID,
		XPC_SERIAL_TYPE_FD,
		XPC_SERIAL_TYPE_SHMEM,
		XPC_SERIAL_TYPE_INVALID, // error
		XPC_SERIAL_TYPE_DOUBLE,
		XPC_SERIAL_TYPE_INVALID, // pointer
	};

	return (xpc_type < (sizeof(xpc_type_map) / sizeof(*xpc_type_map))) ? xpc_type_map[xpc_type] : XPC_SERIAL_TYPE_INVALID;
};

int xpc_serial_write(xpc_serializer_t* serializer, const void* buffer, size_t length, size_t* out_length) {
	size_t total_length = round_up_multiple_of_2(length, 4);
	size_t padding_length = total_length - length;

	if (xpc_serial_space_check(serializer, total_length) < 0)
		return -1;

	if (serializer->buffer && buffer)
		memcpy(serializer->current_position, buffer, length);
	serializer->current_position += length;
	if (serializer->buffer)
		memset(serializer->current_position, 0, padding_length);
	serializer->current_position += padding_length;

	if (out_length)
		*out_length += total_length;

	return 0;
};

int xpc_serial_read(xpc_deserializer_t* deserializer, void* buffer, size_t length, size_t* out_length) {
	size_t total_length = round_up_multiple_of_2(length, 4);

	if (deserializer->current_position + length > deserializer->buffer_end)
		return -1;

	memcpy(buffer, deserializer->current_position, length);
	deserializer->current_position += total_length;

	if (out_length)
		*out_length += total_length;

	return 0;
};

int xpc_serial_write_reserve(xpc_serializer_t* serializer, char** reservation_pointer, size_t reserved_length, size_t* out_length) {
	if (reservation_pointer)
		*reservation_pointer = serializer->buffer ? serializer->current_position : NULL;
	return xpc_serial_write(serializer, NULL, reserved_length, out_length);
};

int xpc_serial_read_in_place(xpc_deserializer_t* deserializer, const void** buffer, size_t length, size_t* out_length) {
	size_t total_length = round_up_multiple_of_2(length, 4);

	*buffer = deserializer->current_position;
	deserializer->current_position += total_length;

	return 0;
};

int xpc_serial_write_string(xpc_serializer_t* serializer, const char* string, size_t* out_length) {
	return xpc_serial_write(serializer, string, strlen(string) + 1, out_length);
};

int xpc_serial_read_string(xpc_deserializer_t* deserializer, const char** string, size_t* out_length) {
	return xpc_serial_read_in_place(deserializer, string, strlen(deserializer->current_position) + 1, out_length);
};

int xpc_serial_write_u32(xpc_serializer_t* serializer, uint32_t integer, size_t* out_length) {
	char data[sizeof(uint32_t)];
	OSWriteLittleInt32(data, 0, integer);
	return xpc_serial_write(serializer, data, sizeof(uint32_t), out_length);
};

int xpc_serial_read_u32(xpc_deserializer_t* deserializer, uint32_t* integer, size_t* out_length) {
	char data[sizeof(uint32_t)];
	if (xpc_serial_read(deserializer, data, sizeof(uint32_t), out_length) < 0)
		return -1;
	*integer = OSReadLittleInt32(data, 0);
	return 0;
};

int xpc_serial_write_u64(xpc_serializer_t* serializer, uint64_t integer, size_t* out_length) {
	char data[sizeof(uint64_t)];
	OSWriteLittleInt64(data, 0, integer);
	return xpc_serial_write(serializer, data, sizeof(uint64_t), out_length);
};

int xpc_serial_read_u64(xpc_deserializer_t* deserializer, uint64_t* integer, size_t* out_length) {
	char data[sizeof(uint64_t)];
	if (xpc_serial_read(deserializer, data, sizeof(uint64_t), out_length) < 0)
		return -1;
	*integer = OSReadLittleInt64(data, 0);
	return 0;
};

int xpc_serial_write_object(xpc_serializer_t* serializer, xpc_object_t object, size_t* out_length) {
	struct xpc_object* xo = object;
	// we don't want to modify `out_length` if we fail, so keep our value local and add it at the end
	size_t local_out_length = 0;

	uint32_t mapped_type = xpc_serial_map_type(xo->xo_xpc_type);

	if (mapped_type < XPC_SERIAL_TYPE_MIN || mapped_type > XPC_SERIAL_TYPE_MAX)
		return -1;

	// write the type
	if (xpc_serial_write_u32(serializer, mapped_type, &local_out_length) < 0)
		return -1;

	switch (xo->xo_xpc_type) {
		case _XPC_TYPE_DICTIONARY: {
			char* reserved_for_size = NULL;
			__block size_t content_size = 0;

			// reserve space for the content size (we'll know it later)
			if (xpc_serial_write_reserve(serializer, &reserved_for_size, sizeof(uint32_t), &local_out_length) < 0)
				return -1;

			// write the number of entries
			if (xpc_serial_write_u32(serializer, xpc_dictionary_get_count(xo), &local_out_length) < 0)
				return -1;

			// write the entries
			if (!xpc_dictionary_apply(xo, ^bool(const char* key, xpc_object_t value) {
				// write the key
				if (xpc_serial_write_string(serializer, key, &content_size) < 0)
					return false;

				// write the content
				if (xpc_serial_write_object(serializer, value, &content_size) < 0)
					return false;

				return true;
			})) {
				return -1;
			}

			// now we write the content size to the space we reserved
			if (reserved_for_size) {
				OSWriteLittleInt32(reserved_for_size, 0, (uint32_t)content_size);
			}

			local_out_length += content_size;
		} break;

		case _XPC_TYPE_ARRAY: {
			char* reserved_for_size = NULL;
			__block size_t content_size = 0;

			// reserve space for the content size (we'll know it later)
			if (xpc_serial_write_reserve(serializer, &reserved_for_size, sizeof(uint32_t), &local_out_length) < 0)
				return -1;

			// write the number of entries
			if (xpc_serial_write_u32(serializer, xpc_array_get_count(xo), &local_out_length) < 0)
				return -1;

			// write the entries
			if (!xpc_array_apply(xo, ^bool(size_t index, xpc_object_t value) {
				if (xpc_serial_write_object(serializer, value, &content_size) < 0)
					return false;

				return true;
			})) {
				return -1;
			}

			// now we write the content size to the space we reserved
			if (reserved_for_size) {
				OSWriteLittleInt32(reserved_for_size, 0, (uint32_t)content_size);
			}

			local_out_length += content_size;
		} break;

		case _XPC_TYPE_BOOL: {
			if (xpc_serial_write_u32(serializer, xpc_bool_get_value(xo), &local_out_length) < 0)
				return -1;
		} break;

		case _XPC_TYPE_INT64: {
			if (xpc_serial_write_u64(serializer, xpc_int64_get_value(xo), &local_out_length) < 0)
				return -1;
		} break;

		case _XPC_TYPE_UINT64: {
			if (xpc_serial_write_u64(serializer, xpc_uint64_get_value(xo), &local_out_length) < 0)
				return -1;
		} break;

		case _XPC_TYPE_NULL: {
			// null doesn't carry a value
		} break;

		case _XPC_TYPE_DOUBLE: {
			// not entirely sure how doubles are supposed to be encoded in Apple's XPC serialization format
			double value = xpc_double_get_value(xo);
			if (xpc_serial_write(serializer, &value, sizeof(double), &local_out_length) < 0)
				return -1;
		} break;

		case _XPC_TYPE_DATA: {
			size_t data_length = xpc_data_get_length(xo);

			if (xpc_serial_write_u32(serializer, data_length, &local_out_length) < 0)
				return -1;

			if (xpc_serial_write(serializer, xpc_data_get_bytes_ptr(xo), data_length, &local_out_length) < 0)
				return -1;
		} break;

		case _XPC_TYPE_STRING: {
			size_t data_length = xpc_string_get_length(xo) + 1;

			if (xpc_serial_write_u32(serializer, data_length, &local_out_length) < 0)
				return -1;

			if (xpc_serial_write(serializer, xpc_string_get_string_ptr(xo), data_length, &local_out_length) < 0)
				return -1;
		} break;

		case _XPC_TYPE_UUID: {
			if (xpc_serial_write(serializer, xpc_uuid_get_bytes(xo), sizeof(uuid_t), &local_out_length) < 0)
				return -1;
		} break;

		case _XPC_TYPE_FD:
		case _XPC_TYPE_SHMEM:
		case _XPC_TYPE_CONNECTION:
		case _XPC_TYPE_ENDPOINT:
		default: {
			debugf("%s:%s:%s: TODO: XPC type %hhu", __FILE__, __LINE__, __FUNCTION__, xo->xo_xpc_type);
			return -1;
		} break;
	}

	if (out_length)
		*out_length += local_out_length;

	return 0;
};

int xpc_serial_read_object(xpc_deserializer_t* deserializer, xpc_object_t* object, size_t* out_length) {
	size_t local_out_length = 0;
	uint32_t type = 0;

	if (xpc_serial_read_u32(deserializer, &type, &local_out_length) < 0)
		return -1;

	switch (type) {
		case XPC_SERIAL_TYPE_NULL: {
			*object = xpc_null_create();
		} break;

		case XPC_SERIAL_TYPE_BOOL: {
			uint32_t value = 0;
			if (xpc_serial_read_u32(deserializer, &value, &local_out_length) < 0)
				return -1;
			*object = xpc_bool_create(value != 0);
		} break;

		case XPC_SERIAL_TYPE_INT64: {
			int64_t value = 0;
			if (xpc_serial_read_u64(deserializer, &value, &local_out_length) < 0)
				return -1;
			*object = xpc_int64_create(value);
		} break;

		case XPC_SERIAL_TYPE_UINT64: {
			uint64_t value = 0;
			if (xpc_serial_read_u64(deserializer, &value, &local_out_length) < 0)
				return -1;
			*object = xpc_uint64_create(value);
		} break;

		case XPC_SERIAL_TYPE_DOUBLE: {
			// again, not sure how to properly represent doubles
			double value = 0;
			if (xpc_serial_read(deserializer, &value, sizeof(double), &local_out_length) < 0)
				return -1;
			*object = xpc_double_create(value);
		} break;

		case XPC_SERIAL_TYPE_DATA: {
			uint32_t size = 0;
			const char* data = NULL;
			if (xpc_serial_read_u32(deserializer, &size, NULL) < 0)
				return -1;
			if (xpc_serial_read_in_place(deserializer, &data, size, &local_out_length) < 0)
				return -1;
			*object = xpc_data_create(data, size);
		} break;

		case XPC_SERIAL_TYPE_STRING: {
			uint32_t size = 0;
			const char* data = NULL;
			if (xpc_serial_read_u32(deserializer, &size, NULL) < 0)
				return -1;
			if (xpc_serial_read_in_place(deserializer, &data, size, &local_out_length) < 0)
				return -1;
			*object = xpc_string_create(data);
		} break;

		case XPC_SERIAL_TYPE_UUID: {
			uuid_t data;
			if (xpc_serial_read(deserializer, &data, sizeof(uuid_t), &local_out_length) < 0)
				return -1;
			*object = xpc_uuid_create(data);
		} break;

		case XPC_SERIAL_TYPE_ARRAY: {
			uint32_t size = 0;
			uint32_t entry_count = 0;

			if (xpc_serial_read_u32(deserializer, &size, NULL) < 0)
				return -1;
			if (xpc_serial_read_u32(deserializer, &entry_count, NULL) < 0)
				return -1;

			// create an empty array; we'll add entries to it as we go
			*object = xpc_array_create(NULL, 0);

			// deserialize all the entries
			for (uint32_t i = 0; i < entry_count; ++i) {
				xpc_object_t entry = NULL;
				if (xpc_serial_read_object(deserializer, &entry, &local_out_length) < 0) {
					xpc_release(*object);
					return -1;
				}
				xpc_array_append_value(*object, entry);
				// retained by the array, so release our ref on it
				xpc_release(entry);
			}
		} break;

		case XPC_SERIAL_TYPE_DICT: {
			uint32_t size = 0;
			uint32_t entry_count = 0;

			if (xpc_serial_read_u32(deserializer, &size, NULL) < 0)
				return -1;
			if (xpc_serial_read_u32(deserializer, &entry_count, NULL) < 0)
				return -1;

			// create an empty dictionary that we'll populate as we go
			*object = xpc_dictionary_create(NULL, NULL, 0);

			for (uint32_t i = 0; i < entry_count; ++i) {
				const char* key = NULL;
				xpc_object_t value = NULL;
				if (xpc_serial_read_string(deserializer, &key, &local_out_length) < 0)
					goto dict_entry_fail;
				if (xpc_serial_read_object(deserializer, &value, &local_out_length) < 0)
					goto dict_entry_fail;

				xpc_dictionary_set_value(*object, key, value);
				// retained by the dictionary, so release our ref on it
				xpc_release(value);

				continue;

				dict_entry_fail:
				xpc_release(*object);
				return -1;
			}
		} break;

		case XPC_SERIAL_TYPE_FD:
		case XPC_SERIAL_TYPE_SHMEM:
		case XPC_SERIAL_TYPE_SEND_PORT:
		case XPC_SERIAL_TYPE_RECV_PORT:
		default: {
			debugf("%s:%s:%s: TODO: XPC type (wire) %u", __FILE__, __LINE__, __FUNCTION__, type);
			return -1;
		} break;
	}

	if (out_length)
		*out_length += local_out_length;

	return 0;
};

int xpc_serialize(xpc_object_t object, void* buffer, size_t length, size_t* out_length) {
	xpc_serializer_t serializer = {
		.buffer = buffer,
		.buffer_end = buffer + length,
		.current_position = buffer,
		.port_count = 0,
		.ports = NULL,
	};

	if (xpc_serial_write_u32(&serializer, XPC_SERIAL_MAGIC, out_length) < 0)
		return -1;

	if (xpc_serial_write_u32(&serializer, XPC_SERIAL_CURRENT_VERSION, out_length) < 0)
		return -1;

	return xpc_serial_write_object(&serializer, object, out_length);
};

int xpc_deserialize(xpc_object_t* object, const void* buffer, size_t length, size_t* out_length) {
	xpc_deserializer_t deserializer = {
		.buffer = buffer,
		.buffer_end = buffer + length,
		.current_position = buffer,
	};
	uint32_t magic = 0;
	uint32_t version = 0;

	if (xpc_serial_read_u32(&deserializer, &magic, out_length) < 0)
		return -1;

	if (magic != XPC_SERIAL_MAGIC)
		return -1;

	if (xpc_serial_read_u32(&deserializer, &version, out_length) < 0)
		return -1;

	if (version != XPC_SERIAL_CURRENT_VERSION)
		return -1;

	return xpc_serial_read_object(&deserializer, object, out_length);
};
