/*
 * Copyright 2014-2015 iXsystems, Inc.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sbuf.h>
#include <mach/mach.h>
#include <xpc/launchd.h>
#include <libkern/OSAtomic.h>
#include <assert.h>
#include <syslog.h>
#include <stdarg.h>
#include <uuid/uuid.h>
#include <stdio.h>
#include "xpc_internal.h"
#include <libkern/OSByteOrder.h>

#define RECV_BUFFER_SIZE	65536

#include <xpc/private.h>

#define sbuf_new_auto() sbuf_new(NULL, NULL, 0, SBUF_AUTOEXTEND)

#define MAX_RECV 8192
#define XPC_RECV_SIZE			\
    MAX_RECV - 				\
    sizeof(mach_msg_header_t) - 	\
    sizeof(mach_msg_trailer_t) - 	\
    sizeof(uint64_t) - 			\
    sizeof(size_t)

const char *const _xpc_event_key_name = "XPCEventName";

struct xpc_message {
	mach_msg_header_t header;
	size_t size;
	uint64_t id;
	char data[0];
	mach_msg_trailer_t trailer;
};

struct xpc_recv_message {
	mach_msg_header_t header;
	size_t size;
	uint64_t id;
	char data[XPC_RECV_SIZE];
	mach_msg_trailer_t trailer;
};

static void xpc_copy_description_level(xpc_object_t obj, struct sbuf *sbuf,
    int level);

void
fail_log(const char *exp)
{
	debugf("%s", exp);
	//sleep(1);
	printf("%s", exp);
	//abort();
}

static void
xpc_dictionary_destroy(struct xpc_object *dict)
{
	struct xpc_dict_head *head;
	struct xpc_dict_pair *p, *ptmp;

	head = &dict->xo_dict;

	TAILQ_FOREACH_SAFE(p, head, xo_link, ptmp) {
		TAILQ_REMOVE(head, p, xo_link);
		free(p->key);
		xpc_object_destroy(p->value);
		free(p);
	}
}

static void
xpc_array_destroy(struct xpc_object *dict)
{
	struct xpc_object *p, *ptmp;
	struct xpc_array_head *head;

	head = &dict->xo_array;

	TAILQ_FOREACH_SAFE(p, head, xo_link, ptmp) {
		TAILQ_REMOVE(head, p, xo_link);
		xpc_object_destroy(p);
	}
}

/**
 * encoded buffer layout for different types:
 * 	dictionary = { 0x01, { key, encode_entry(encode_size(sizeof(value))?, value) }... }
 * 	array      = { 0x02, encode_entry(encode_size(sizeof(entry))?, entry)... }
 * 	bool       = { 0x03 | ((value ? 0x01 : 0x00) << XPC_PACK_TYPE_BITS) }
 * 	connection = { 0x04 }
 * 	endpoint   = { 0x05 }
 * 	null       = { 0x06 }
 * 	activity   = { 0x07 }
 * 	int64      = { encode_int(0x08, value, sizeof(int64_t)) }
 * 	uint64     = { encode_int(0x09, value, sizeof(uint64_t)) }
 * 	date       = { encode_int(0x0a, value, sizeof(int64_t)) }
 * 	data       = { 0x0b, values... }
 * 	string     = { 0x0c, bytes... }
 * 	uuid       = { encode_int(0x0d, value, 16) }
 * 	fd         = { encode_int(0x0e, value, sizeof(int)) }
 * 	shmem      = { 0x0f }
 * 	error      = { 0x10 }
 * 	double     = { encode_int(0x11, value, sizeof(double)) }
 *
 * where:
 * 	sizeof(x)                       = length of x in bytes
 * 	encode_int(type, x, max_bytes)  = encodes the provided integer into the minimum number of bytes
 * 	                                  possible. the high bit on each byte specifies whether the value has another byte.
 * 	                                  an optimization is made that uses the unused bits in the type byte to encode the value.
 * 	                                  encodes in little-endian.
 * 	encode_size(x)                  = encodes the provided integer into the minimum number of bytes
 * 	                                  possible, much like `encode_int`. the difference is that it doesn't automatically
 * 	                                  encode the type into the first byte.
 * 	                                  encodes in little-endian.
 * 	encode_entry(size, value)       = if the value is variable-length type:
 * 	                                  	{ size[0] | value[0], size[1..], value[1..] }
 * 	                                  otherwise:
 * 	                                  	{ value }
 *
 * notes:
 * 	* integral values are laid out in little-endian
 * 	* many types that have variable length do not automatically include a stated length
 * 		* this is done to save precious bytes
 * 		* the only time size is explicitly specified is when variable-size structures are used
 * 		  inside other variable-size structures (e.g. dictionary in an array, data in a dictionary, array in an array, etc.).
 * 		* this rule does *not* apply to strings and integral values, as they are technically not considered "variable-length"
 * 			* strings are null-terminated, so it's obvious when you've reached their end
 * 			* integral values encoded with encode_int likewise also already carry an indiciation
 * 			  of their length as part of their format
 * 	* there may have been additional size optimization that could've been made that i didn't know about,
 * 	  but i was also trying to find a balance between size-efficiency, computational-efficiency, and simplicity.
 */

#define XPC_PACK_TYPE_MASK 0x1f
#define XPC_PACK_TYPE_BITS 5
#define XPC_PACK_INT_INITIAL_BITS (8 - (XPC_PACK_TYPE_BITS + 1))
#define XPC_PACK_INT_EXTRA_BYTES(x) (((x - (8 - XPC_PACK_TYPE_BITS)) + 7) / 8)

XPC_INLINE bool xpc_pack_is_variable_length(uint8_t xo_xpc_type) {
	return xo_xpc_type == _XPC_TYPE_DICTIONARY || xo_xpc_type == _XPC_TYPE_ARRAY || xo_xpc_type == _XPC_TYPE_DATA;
};

XPC_INLINE uint8_t xpc_pack_generate_mask(uint8_t bits, bool least_significant) {
	static const uint8_t lookup_table[] = {
		0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
	};
	return lookup_table[bits] >> (least_significant ? (8 - bits) : 0);
};

// buf must be little-endian
XPC_INLINE size_t xpc_pack_encode_int_bits(const uint8_t* buf, size_t buf_size, bool raw) {
	while (buf_size > 0 && buf[buf_size - 1] == 0)
		--buf_size;

	if (buf_size == 0)
		return 0;

	uint8_t msb = buf[buf_size - 1];
	size_t leading_bit_count = 8;
	for (uint8_t i = 8; i > 0; --i) {
		if ((msb & (1 << (i - 1))) == 0) {
			--leading_bit_count;
		} else {
			break;
		}
	}

	// number of bits needed for actual number
	size_t raw_bits = ((buf_size - 1) * 8) + leading_bit_count;

	if (raw)
		return raw_bits;

	if (raw_bits <= XPC_PACK_INT_INITIAL_BITS)
		return XPC_PACK_INT_INITIAL_BITS + 1;

	return raw_bits + ((raw_bits - XPC_PACK_INT_INITIAL_BITS) / 7) + 1;
};

// buf must be little-endian
XPC_INLINE void xpc_pack_encode_int(uint8_t* out, const uint8_t* buf, size_t buf_size) {
	if (buf_size == 0)
		return;

	uint8_t initial_mask = xpc_pack_generate_mask(8 - XPC_PACK_TYPE_BITS - 1, true);
	out[0] = ((buf[0] & initial_mask) << XPC_PACK_TYPE_BITS) | out[0] & XPC_PACK_TYPE_MASK;

	if ((buf[0] & ~initial_mask) == 0)
		return;

	out[0] |= 1 << 7;

	size_t raw_bits = xpc_pack_encode_int_bits(buf, buf_size, true) - (8 - (XPC_PACK_TYPE_BITS + 1));
	size_t leftover_bits = XPC_PACK_TYPE_BITS + 1;
	size_t buf_idx = 0;
	size_t out_idx = 1;

	for (; raw_bits >= 7; raw_bits -= 7, ++out_idx) {
		if (leftover_bits == 8) {
			// 0x7f == 0b01111111
			out[out_idx] = (1 << 7) | buf[buf_idx] & 0x7f;
			leftover_bits = 1;
		} else {
			uint8_t prev_byte_mask = xpc_pack_generate_mask(leftover_bits, false);
			uint8_t bits_left_in_encoding_segment = 7 - leftover_bits;
			uint8_t next_byte_mask = xpc_pack_generate_mask(bits_left_in_encoding_segment, true);
			out[out_idx] = (1 << 7) | ((buf[buf_idx] & prev_byte_mask) >> (8 - leftover_bits)) | ((buf[buf_idx + 1] & next_byte_mask) << leftover_bits);
			leftover_bits = 8 - bits_left_in_encoding_segment;
			++buf_idx;
		}
	}

	if (raw_bits == 0) {
		// no next byte; remove the indicator
		// 0x7f == 0b01111111
		out[out_idx - 1] &= 0x7f;
	} else {
		uint8_t prev_byte_mask = xpc_pack_generate_mask(leftover_bits, false);
		out[out_idx] = (buf[buf_idx] & prev_byte_mask) >> (8 - leftover_bits);
		if (leftover_bits < raw_bits) {
			uint8_t bits_left_in_encoding_segment = 7 - leftover_bits;
			uint8_t next_byte_mask = xpc_pack_generate_mask(bits_left_in_encoding_segment, true);
			out[out_idx] |= (buf[buf_idx + 1] & next_byte_mask) << leftover_bits;
		}
	}
};

XPC_INLINE size_t xpc_unpack_decode_int(uint8_t* out, const uint8_t* buf) {
	// `>> 1` because we don't want to include the most significant bit
	uint8_t initial_mask = xpc_pack_generate_mask(8 - XPC_PACK_TYPE_BITS - 1, false) >> 1;
	out[0] = (buf[0] & initial_mask) >> XPC_PACK_TYPE_BITS;

	if ((buf[0] & (1 << 7)) == 0)
		return 1;

	size_t leftover_bits = XPC_PACK_TYPE_BITS + 1;
	size_t out_idx = 0;
	size_t buf_idx = 1;

	while ((buf[buf_idx] & (1 << 7)) != 0) {
		if (leftover_bits == 8) {
			// 0x7f == 0b01111111
			out[out_idx] = buf[buf_idx] & 0x7f;
			leftover_bits = 1;
		} else {
			uint8_t prev_byte_mask = xpc_pack_generate_mask(leftover_bits, true);
			uint8_t bits_left_in_encoding_segment = 7 - leftover_bits;
			uint8_t next_byte_mask = xpc_pack_generate_mask(bits_left_in_encoding_segment, false) >> 1;
			out[out_idx] |= (buf[buf_idx] & prev_byte_mask) << (8 - leftover_bits);
			out[out_idx + 1] = (buf[buf_idx] & next_byte_mask) >> leftover_bits;
			leftover_bits = 8 - bits_left_in_encoding_segment;
			++out_idx;
		}
		++buf_idx;
	}

	uint8_t prev_byte_mask = xpc_pack_generate_mask(leftover_bits, true);
	uint8_t bits_left_in_encoding_segment = 7 - leftover_bits;
	uint8_t next_byte_mask = xpc_pack_generate_mask(bits_left_in_encoding_segment, false) >> 1;
	out[out_idx] |= (buf[buf_idx] & prev_byte_mask) << (8 - leftover_bits);
	if ((buf[buf_idx] & next_byte_mask) != 0) {
		out[out_idx + 1] = (buf[buf_idx] & next_byte_mask) >> leftover_bits;
	}

	return buf_idx + 1;
};

XPC_INLINE int xpc_pack(struct xpc_object* xo, void* buf, size_t* final_size);
XPC_INLINE struct xpc_object* xpc_unpack(const void* buf, size_t size, size_t* packed_size, uint8_t declared_type);

XPC_INLINE void xpc_pack_encode_entry(uint8_t* out, size_t* size, size_t* offset, struct xpc_object* val_xo) {
	size_t val_size = 0;
	xpc_pack(val_xo, out ? out + *offset : out, &val_size);
	if (xpc_pack_is_variable_length(val_xo->xo_xpc_type)) {
		size_t val_size_le = sizeof(size_t) == 4 ? OSSwapHostToLittleInt32(val_size) : OSSwapHostToLittleInt64(val_size);
		size_t size_bits = xpc_pack_encode_int_bits(&val_size_le, sizeof(size_t), false);
		size_t size_size = XPC_PACK_INT_EXTRA_BYTES(size_bits);
		if (out) {
			if (size_size > 0 && val_size > 1)
				memmove(out + *offset + 1 + size_size, out + *offset + 1, val_size);
			xpc_pack_encode_int(out + *offset, &val_size_le, sizeof(size_t));
		}
		*size += size_size;
		if (out)
			*offset += size_size;
	}
	*size += val_size;
	if (out)
		*offset += val_size;
};

XPC_INLINE struct xpc_object* xpc_unpack_decode_entry(const uint8_t* in, size_t size, size_t* offset) {
	struct xpc_object* xo = NULL;
	uint8_t value_type = in[*offset] & XPC_PACK_TYPE_MASK;
	size_t val_packed_size = 0;
	if (xpc_pack_is_variable_length(value_type)) {
		size_t val_size_le = 0;
		// `- 1` because part of the size is encoded into the type byte
		size_t val_size_size = xpc_unpack_decode_int(&val_size_le, in + *offset) - 1;
		size_t val_size = sizeof(size_t) == 4 ? OSSwapLittleToHostInt32(val_size_le) : OSSwapLittleToHostInt64(val_size_le);
		*offset += val_size_size;
		xo = xpc_unpack(in + *offset, val_size, &val_packed_size, value_type);
		if (val_packed_size != val_size) {
			// something's up
			debugf("val_packed_size != val_size (%zu != %zu)", val_packed_size, val_size);
			return NULL;
		}
	} else {
		xo = xpc_unpack(in + *offset, size - *offset, &val_packed_size, 0);
	}
	*offset += val_packed_size;
	return xo;
};

XPC_INLINE int xpc_pack(struct xpc_object* xo, void* buf, size_t* final_size) {
	uint8_t* out = buf;
	__block size_t size = 1;

	if (out)
		out[0] = xo->xo_xpc_type;

	switch (xo->xo_xpc_type) {
		case _XPC_TYPE_DICTIONARY: {
			__block size_t offset = size;
			xpc_dictionary_apply(xo, ^bool(const char* key, xpc_object_t value) {
				struct xpc_object* val_xo = value;
				size_t key_size = strlen(key) + 1;
				size += key_size;
				if (out) {
					strncpy(out + offset, key, key_size - 1);
					offset += key_size;
					out[offset - 1] = '\0';
				}
				xpc_pack_encode_entry(out, &size, &offset, val_xo);
				return true;
			});
		} break;
		case _XPC_TYPE_ARRAY: {
			__block size_t offset = size;
			xpc_array_apply(xo, ^bool(size_t idx, xpc_object_t value) {
				struct xpc_object* val_xo = value;
				xpc_pack_encode_entry(out, &size, &offset, val_xo);
				return true;
			});
		} break;
		case _XPC_TYPE_BOOL: {
			if (out)
				out[0] |= (xpc_bool_get_value(xo) ? 1 : 0) << XPC_PACK_TYPE_BITS;
		} break;
		case _XPC_TYPE_DATE:
		case _XPC_TYPE_INT64: {
			int64_t val = xo->xo_xpc_type == _XPC_TYPE_DATE ? xpc_date_get_value(xo) : xpc_int64_get_value(xo);
			int64_t val_le = OSSwapHostToLittleInt64(val);
			size_t val_bits = xpc_pack_encode_int_bits(&val_le, sizeof(int64_t), false);
			size += XPC_PACK_INT_EXTRA_BYTES(val_bits);
			if (out)
				xpc_pack_encode_int(out, &val_le, sizeof(int64_t));
		} break;
		case _XPC_TYPE_UINT64: {
			uint64_t val = xpc_uint64_get_value(xo);
			uint64_t val_le = OSSwapHostToLittleInt64(val);
			size_t val_bits = xpc_pack_encode_int_bits(&val_le, sizeof(uint64_t), false);
			size += XPC_PACK_INT_EXTRA_BYTES(val_bits);
			if (out)
				xpc_pack_encode_int(out, &val_le, sizeof(uint64_t));
		} break;
		case _XPC_TYPE_DATA: {
			size_t len = xpc_data_get_length(xo);
			size += len;
			if (out)
				memcpy(out + 1, xpc_data_get_bytes_ptr(xo), len);
		} break;
		case _XPC_TYPE_STRING: {
			size_t len = xpc_string_get_length(xo);
			size += len + 1;
			if (out) {
				strncpy(out + 1, xpc_string_get_string_ptr(xo), len);
				out[1 + len] = '\0';
			}
		} break;
		// currently unimplemented
		//case _XPC_TYPE_UUID: {} break;
		case _XPC_TYPE_FD: {
			int val = xo->xo_u.port;
			int val_le = OSSwapHostToLittleInt32(val);
			size_t val_bits = xpc_pack_encode_int_bits(&val_le, sizeof(int), false);
			size += XPC_PACK_INT_EXTRA_BYTES(val_bits);
			if (out)
				xpc_pack_encode_int(out, &val_le, sizeof(int));
		} break;
		case _XPC_TYPE_DOUBLE: {
			// this is a rather simplistic implementation that assumes that the internal layout
			// of a double is the same on sender and reeceiver
			// that's never a good assumption.
			// (although from what i can tell, XPC is only used for communication between processes
			// on the same system, so this shouldn't be a problem)
			//
			// TODO: figure out how to encode the double portably
			double val = xpc_double_get_value(xo);
			uint64_t val_le = sizeof(double) == 4 ? (uint64_t)OSSwapHostToLittleInt32(*(uint32_t*)&val) : OSSwapHostToLittleInt64(*(uint64_t*)&val);
			size_t val_bits = xpc_pack_encode_int_bits(&val_le, sizeof(uint64_t), false);
			size += XPC_PACK_INT_EXTRA_BYTES(val_bits);
			if (out)
				xpc_pack_encode_int(out, &val_le, sizeof(uint64_t));
		} break;
		case _XPC_TYPE_NULL: break;
		default: {
			debugf("can't pack unsupported type: %d", xo->xo_xpc_type);
		} break;
	};

	if (final_size)
		*final_size = size;

	return 0;
};

// initial calls (i.e. anyone using this function) should call it like so:
// 	xpc_unpack(data, data_size, NULL, 0)
// or
// 	xpc_unpack(data, data_size, &packed_size, 0)
// in other words, the `declared_type` parameter should always be 0
XPC_INLINE struct xpc_object* xpc_unpack(const void* buf, size_t size, size_t* packed_size, uint8_t declared_type) {
	if (size == 0) {
		if (packed_size)
			*packed_size = 0;
		return NULL;
	}

	const uint8_t* in = buf;
	uint8_t xo_xpc_type = declared_type == 0 ? in[0] & XPC_PACK_TYPE_MASK : declared_type;
	size_t offset = 0;
	struct xpc_object* xo = NULL;

	switch (xo_xpc_type) {
		case _XPC_TYPE_DICTIONARY: {
			xo = xpc_dictionary_create(NULL, NULL, 0);
			++offset;

			while (offset < size) {
				const char* key = in + offset;
				size_t key_size = strlen(key) + 1;
				offset += key_size;
				struct xpc_object* val = xpc_unpack_decode_entry(in, size, &offset);
				if (!val) {
					debugf("failed to unpack dictionary entry");
					xpc_release(xo);
					xo = NULL;
					offset = 0;
					break;
				}
				xpc_dictionary_set_value(xo, key, val);
				xpc_release(val);
			}
		} break;
		case _XPC_TYPE_ARRAY: {
			xo = xpc_array_create(NULL, 0);
			++offset;

			while (offset < size) {
				struct xpc_object* val = xpc_unpack_decode_entry(in, size, &offset);
				if (!val) {
					debugf("failed to unpack array entry");
					xpc_release(xo);
					xo = NULL;
					offset = 0;
					break;
				}
				xpc_array_append_value(xo, val);
				xpc_release(val);
			}
		} break;
		case _XPC_TYPE_BOOL: {
			xo = xpc_bool_create(in[0] >> XPC_PACK_TYPE_BITS);
			++offset;
		} break;
		case _XPC_TYPE_DATE:
		case _XPC_TYPE_INT64: {
			int64_t val_le = 0;
			offset += xpc_unpack_decode_int(&val_le, in + offset);
			int64_t val = OSSwapLittleToHostInt64(val_le);
			xo = xo_xpc_type == _XPC_TYPE_DATE ? xpc_date_create(val) : xpc_int64_create(val);
		} break;
		case _XPC_TYPE_UINT64: {
			uint64_t val_le = 0;
			offset += xpc_unpack_decode_int(&val_le, in + offset);
			uint64_t val = OSSwapLittleToHostInt64(val_le);
			xo = xpc_uint64_create(val);
		} break;
		case _XPC_TYPE_DATA: {
			++offset;
			xo = xpc_data_create(in + offset, size - offset);
			offset += size - offset;
		} break;
		case _XPC_TYPE_STRING: {
			++offset;
			xo = xpc_string_create(in + offset);
			offset += strlen(in + offset) + 1;
		} break;
		case _XPC_TYPE_FD: {
			int val_le = 0;
			offset += xpc_unpack_decode_int(&val_le, in + offset);
			int val = OSSwapLittleToHostInt32(val_le);

			xo = malloc(sizeof(struct xpc_object));
			if (xo) {
				xo->xo_size = sizeof(xo->xo_u.port);
				xo->xo_xpc_type = _XPC_TYPE_FD;
				xo->xo_flags = 0;
				xo->xo_u.port = val;
				xo->xo_refcnt = 1;
				xo->xo_audit_token = NULL;
			}
		} break;
		case _XPC_TYPE_DOUBLE: {
			uint64_t uval_le = 0;
			offset += xpc_unpack_decode_int(&uval_le, in + offset);
			uint64_t uval = OSSwapLittleToHostInt64(uval_le);
			double val = *(double*)&uval_le;
			xo = xpc_double_create(val);
		} break;
		case _XPC_TYPE_NULL: {
			xo = xpc_null_create();
			++offset;
		} break;
		default: {
			debugf("can't unpack unsupported type: %d", xo_xpc_type);
		} break;
	}

	if (packed_size)
		*packed_size = offset;

	return xo;
};

void
xpc_object_destroy(struct xpc_object *xo)
{
	if (xo->xo_refcnt == _XPC_KEEP_ALIVE)
		return;

	if (xo->xo_xpc_type == _XPC_TYPE_DICTIONARY)
		xpc_dictionary_destroy(xo);

	if (xo->xo_xpc_type == _XPC_TYPE_ARRAY)
		xpc_array_destroy(xo);

	if (xo->xo_xpc_type == _XPC_TYPE_CONNECTION)
		xpc_connection_destroy(xo);

	if (xo->xo_xpc_type == _XPC_TYPE_STRING)
		free(xo->xo_u.str);

	if (xo->xo_xpc_type == _XPC_TYPE_DATA)
		free(xo->xo_u.ptr);

	free(xo);
}

xpc_object_t
xpc_retain(xpc_object_t obj)
{
	struct xpc_object *xo = obj;
	if (xo->xo_refcnt == _XPC_KEEP_ALIVE)
		return obj;

	OSAtomicIncrement32(&xo->xo_refcnt);
	return (obj);
}

void
xpc_release(xpc_object_t obj)
{
	struct xpc_object *xo = obj;
	if (xo->xo_refcnt == _XPC_KEEP_ALIVE)
		return;

	if (OSAtomicDecrement32(&xo->xo_refcnt) > 0)
		return;

	xpc_object_destroy(xo);
}

// The other one is unsafe?
// This is called by Security and is private
void
xpc_release_safe(xpc_object_t obj)
{
	xpc_release(obj);
}

static const char *xpc_errors[] = {
	"No Error Found",
	"No Memory",
	"Invalid Argument",
	"No Such Process"
};


const char *
xpc_strerror(int error)
{

	if (error > EXMAX || error < 0)
		return "BAD ERROR";
	return (xpc_errors[error]);
}

char *
xpc_copy_description(xpc_object_t obj)
{
	char *result;
	struct sbuf *sbuf;

	sbuf = sbuf_new_auto();
	xpc_copy_description_level(obj, sbuf, 0);
	sbuf_finish(sbuf);
	result = strdup(sbuf_data(sbuf));
	sbuf_delete(sbuf);

	return (result);
}

static void
xpc_copy_description_level(xpc_object_t obj, struct sbuf *sbuf, int level)
{
	struct xpc_object *xo = obj;
#ifndef __APPLE__
	struct uuid *id;
#else
	uuid_t id;
#endif
	char *uuid_str;
	uint32_t uuid_status;

	if (obj == NULL) {
		sbuf_printf(sbuf, "<null value>\n");
		return;
	}

	sbuf_printf(sbuf, "(%s) ", _xpc_get_type_name(obj));

	switch (xo->xo_xpc_type) {
	case _XPC_TYPE_DICTIONARY:
		sbuf_printf(sbuf, "\n");
		xpc_dictionary_apply(xo, ^(const char *k, xpc_object_t v) {
			sbuf_printf(sbuf, "%*s\"%s\": ", level * 4, " ", k);
			xpc_copy_description_level(v, sbuf, level + 1);
			return ((bool)true);
		});
		break;

	case _XPC_TYPE_ARRAY:
		sbuf_printf(sbuf, "\n");
		xpc_array_apply(xo, ^(size_t idx, xpc_object_t v) {
			sbuf_printf(sbuf, "%*s%ld: ", level * 4, " ", idx);
			xpc_copy_description_level(v, sbuf, level + 1);
			return ((bool)true);
		});
		break;

	case _XPC_TYPE_BOOL:
		sbuf_printf(sbuf, "%s\n",
		    xpc_bool_get_value(obj) ? "true" : "false");
		break;

	case _XPC_TYPE_STRING:
		sbuf_printf(sbuf, "\"%s\"\n",
		    xpc_string_get_string_ptr(obj));
		break;

	case _XPC_TYPE_INT64:
		sbuf_printf(sbuf, "%lld\n",
		    xpc_int64_get_value(obj));
		break;

	case _XPC_TYPE_UINT64:
		sbuf_printf(sbuf, "0x%llx\n",
		    xpc_uint64_get_value(obj));
		break;

	case _XPC_TYPE_DATE:
		sbuf_printf(sbuf, "%llu\n",
		    xpc_date_get_value(obj));
		break;	

	case _XPC_TYPE_UUID:
#ifdef __APPLE__
		memcpy(id, xpc_uuid_get_bytes(obj), sizeof(id));
		uuid_str = (char*) __builtin_alloca(40);
		uuid_unparse(*id, uuid_str);
#else
		id = (struct uuid *)xpc_uuid_get_bytes(obj);
		uuid_to_string(id, &uuid_str, &uuid_status);
#endif
		sbuf_printf(sbuf, "%s\n", uuid_str);
		free(uuid_str);
		break;

	case _XPC_TYPE_ENDPOINT:
		sbuf_printf(sbuf, "<%d>\n", xo->xo_int);
		break;

	case _XPC_TYPE_NULL:
		sbuf_printf(sbuf, "<null>\n");
		break;
	}
}

struct _launch_data {
	uint64_t type;
	union {
		struct {
			union {
				launch_data_t *_array;
				char *string;
				void *opaque;
				int64_t __junk;
			};
			union {
				uint64_t _array_cnt;
				uint64_t string_len;
				uint64_t opaque_size;
			};
		};
		int64_t fd;
		uint64_t  mp;
		uint64_t err;
		int64_t number;
		uint64_t boolean; /* We'd use 'bool' but this struct needs to be used under Rosetta, and sizeof(bool) is different between PowerPC and Intel */
		double float_num;
	};
};

static uint8_t ld_to_xpc_type[] = {
	_XPC_TYPE_INVALID,
	_XPC_TYPE_DICTIONARY,
	_XPC_TYPE_ARRAY,
	_XPC_TYPE_FD,
	_XPC_TYPE_UINT64,
	_XPC_TYPE_DOUBLE,
	_XPC_TYPE_BOOL,
	_XPC_TYPE_STRING,
	_XPC_TYPE_DATA,
	_XPC_TYPE_ERROR,
	_XPC_TYPE_ENDPOINT
};
	
xpc_object_t
ld2xpc(launch_data_t ld)
{
	struct xpc_object *xo;
	xpc_u val;


	if (ld->type > LAUNCH_DATA_MACHPORT)
		return (NULL);
	if (ld->type == LAUNCH_DATA_STRING || ld->type == LAUNCH_DATA_OPAQUE) {
		val.str = malloc(ld->string_len);
		memcpy(__DECONST(void *, val.str), ld->string, ld->string_len);
		xo = _xpc_prim_create(ld_to_xpc_type[ld->type], val, ld->string_len);
	} else if (ld->type == LAUNCH_DATA_BOOL) {
		xo = xpc_bool_create((bool)ld->boolean);
	} else if (ld->type == LAUNCH_DATA_ARRAY) {
		xo = xpc_array_create(NULL, 0);
		for (uint64_t i = 0; i < ld->_array_cnt; i++)
			xpc_array_append_value(xo, ld2xpc(ld->_array[i]));
	} else {
		val.ui = ld->mp;
		xo = _xpc_prim_create(ld_to_xpc_type[ld->type], val, ld->string_len);	
	}
	return (xo);
}

xpc_object_t
xpc_copy_entitlement_for_token(const char *key __unused, audit_token_t *token __unused)
{
	return xpc_bool_create(true);
}

xpc_object_t
xpc_copy_entitlements_for_pid(pid_t pid)
{
	return xpc_bool_create(true);
}


#define XPC_RPORT "XPC remote port"
int
xpc_pipe_routine_reply(xpc_object_t xobj)
{
	struct xpc_object *xo;
	size_t size, msg_size;
	struct xpc_message *message;
	kern_return_t kr;
	int err;

	xo = xobj;
	assert(xo->xo_xpc_type == _XPC_TYPE_DICTIONARY);

	if (xpc_pack(xo, NULL, &size) != 0)
        return errno;

	msg_size = size + sizeof(struct xpc_message);

	if ((message = malloc(msg_size)) == NULL)
		return ENOMEM;

	if (xpc_pack(xo, message->data, &size) != 0)
        return errno;

	message->header.msgh_size = msg_size;
	message->header.msgh_remote_port = xpc_dictionary_copy_mach_send(xobj, XPC_RPORT);
	message->header.msgh_local_port = MACH_PORT_NULL;
	message->size = size;
	kr = mach_msg_send(&message->header);
	if (kr != KERN_SUCCESS)
		err = (kr == KERN_INVALID_TASK) ? EPIPE : EINVAL;
	else
		err = 0;
	free(message);
	return (err);
}

int
xpc_pipe_send(xpc_object_t xobj, mach_port_t dst, mach_port_t local,
    uint64_t id)
{
	struct xpc_object *xo;
	size_t size, msg_size;
	struct xpc_message *message;
	kern_return_t kr;
	int err;

	xo = xobj;
	debugf("obj type is %d", xo->xo_xpc_type);
	if (xo->xo_xpc_type != _XPC_TYPE_DICTIONARY)
		debugf("obj type is %s", _xpc_get_type_name(xobj));
	assert(xo->xo_xpc_type == _XPC_TYPE_DICTIONARY);

	debugf("packing message");
	if (xpc_pack(xo, NULL, &size) != 0)
		return errno;

	msg_size = size + sizeof(struct xpc_message);
	if ((message = malloc(msg_size)) == NULL)
		return ENOMEM;

	if (xpc_pack(xo, message->data, &size) != 0)
		return errno;

	debugf("sending message");
	msg_size = ALIGN(size + sizeof(mach_msg_header_t) + sizeof(size_t) + sizeof(uint64_t));
	message->header.msgh_size = (mach_msg_size_t)msg_size;
	message->header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,
	    MACH_MSG_TYPE_MAKE_SEND);
	message->header.msgh_remote_port = dst;
	message->header.msgh_local_port = local;
	message->id = id;
	message->size = size;
	kr = mach_msg_send(&message->header);
	if (kr != KERN_SUCCESS)
		err = (kr == KERN_INVALID_TASK) ? EPIPE : EINVAL;
	else
		err = 0;
	free(message);
	return (err);	
}

#define LOG(msg, ...)	\
	do {            \
	debugf("%s:%u: " msg, __FILE__, __LINE__,##__VA_ARGS__);	\
	} while (0)

int
xpc_pipe_receive(mach_port_t local, mach_port_t *remote, xpc_object_t *result,
    uint64_t *id)
{
	struct xpc_recv_message message;
	mach_msg_header_t *request;
	kern_return_t kr;
	mach_msg_trailer_t *tr;
	int data_size;
	struct xpc_object *xo;
	audit_token_t *auditp;

	request = &message.header;
	/* should be size - but what about arbitrary XPC data? */
	request->msgh_size = MAX_RECV;
	request->msgh_local_port = local;
	kr = mach_msg(request, MACH_RCV_MSG |
	    MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) |
	    MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT),
	    0, request->msgh_size, request->msgh_local_port,
	    MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);

	if (kr != 0)
		LOG("mach_msg_receive returned %d\n", kr);
	*remote = request->msgh_remote_port;
	*id = message.id;
	data_size = (int)message.size;
	LOG("unpacking data_size=%d", data_size);
	xo = xpc_unpack(message.data, data_size, NULL, 0);

	tr = (mach_msg_trailer_t *)(((char *)&message) + request->msgh_size);
	auditp = &((mach_msg_audit_trailer_t *)tr)->msgh_audit;

	xo->xo_audit_token = malloc(sizeof(*auditp));
	memcpy(xo->xo_audit_token, auditp, sizeof(*auditp));

	xpc_dictionary_set_mach_send(xo, XPC_RPORT, request->msgh_remote_port);
	xpc_dictionary_set_uint64(xo, XPC_SEQID, message.id);
	xo->xo_flags |= _XPC_FROM_WIRE;

	*result = xo;
	return (0);
}

int
xpc_pipe_try_receive(mach_port_t portset, xpc_object_t *requestobj, mach_port_t *rcvport,
	boolean_t (*demux)(mach_msg_header_t *, mach_msg_header_t *), mach_msg_size_t msgsize __unused,
	int flags __unused)
{
	struct xpc_recv_message message;
	struct xpc_recv_message rsp_message;
	mach_msg_header_t *request;
	kern_return_t kr;
	mach_msg_header_t *response;
	mach_msg_trailer_t *tr;
	int data_size;
	struct xpc_object *xo;
	audit_token_t *auditp;

	request = &message.header;
	response = &rsp_message.header;
	/* should be size - but what about arbitrary XPC data? */
	request->msgh_size = MAX_RECV;
	request->msgh_local_port = portset;
	kr = mach_msg(request, MACH_RCV_MSG |
	    MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) |
	    MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT),
	    0, request->msgh_size, request->msgh_local_port,
	    MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);

	if (kr != 0)
		LOG("mach_msg_receive returned %d\n", kr);
	*rcvport = request->msgh_remote_port;
	if (demux(request, response)) {
		mig_reply_error_t* migError = (mig_reply_error_t*) response;

		if (!(migError->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
			if (migError->RetCode == MIG_NO_REPLY)
				migError->Head.msgh_remote_port = MACH_PORT_NULL;
		}

		if (response->msgh_remote_port != MACH_PORT_NULL)
			(void)mach_msg_send(response);
		/*  can't do anything with the return code
		* just tell the caller this has been handled
		*/
		return (TRUE);
	}
	LOG("demux returned false\n");
	data_size = request->msgh_size;
	LOG("unpacking data_size=%d", data_size);
	xo = xpc_unpack(message.data, data_size, NULL, 0);
	/* is padding for alignment enforced in the kernel?*/
	tr = (mach_msg_trailer_t *)(((char *)&message) + request->msgh_size);
	auditp = &((mach_msg_audit_trailer_t *)tr)->msgh_audit;

	xo->xo_audit_token = malloc(sizeof(*auditp));
	memcpy(xo->xo_audit_token, auditp, sizeof(*auditp));

	xpc_dictionary_set_mach_send(xo, XPC_RPORT, request->msgh_remote_port);
	xpc_dictionary_set_uint64(xo, XPC_SEQID, message.id);
	xo->xo_flags |= _XPC_FROM_WIRE;
	*requestobj = xo;
	return (0);
}

int
xpc_call_wakeup(mach_port_t rport, int retcode)
{
	mig_reply_error_t msg;
	int err;
	kern_return_t kr;

	msg.Head.msgh_remote_port = rport;
	msg.RetCode = retcode;
	kr = mach_msg_send(&msg.Head);
	if (kr != KERN_SUCCESS)
		err = (kr == KERN_INVALID_TASK) ? EPIPE : EINVAL;
	else
		err = 0;

	return (err);
}

xpc_object_t
_od_rpc_call(const char *procname, xpc_object_t payload, xpc_pipe_t (*get_pipe)(bool))
{
	printf("STUB _od_rpc_call\n");
	return NULL;
}

int
xpc_pipe_routine(xpc_object_t pipe, void *payload,xpc_object_t *reply)
{
	printf("STUB xpc_pipe_routine\n");
	return 0;
}

void
xpc_dictionary_set_uuid(xpc_object_t xdict, const char *key, const uuid_t uuid)
{
	printf("STUB xpc_dictionary_set_uuid\n");
}

int launch_activate_socket(const char* key, int** fds, size_t* count) {
	// notes for someone implementing this in the future:
	//
	// this function is used in OpenSSH in ssh-agent.c
	//
	// `key` is the socket key in the current process's launchd plist
	// `fds` is a pointer to an array that we allocate that is freed by the caller
	// `count` is the size of that array
	//
	// implementing this requires looking up the current process's launchd plist
	// and reading socket values from there (or talking to launchd to do that,
	// if we can do that; i haven't looked into this much)
	printf("STUB launch_activate_socket\n");
	if (fds)
		*fds = NULL;
	if (count)
		*count = 0;
	return -1;
};
