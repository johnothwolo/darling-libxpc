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

#import <xpc/objects/uuid.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/serialization.h>

#define UUID_STRING_LENGTH 36

XPC_CLASS_SYMBOL_DECL(uuid);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(uuid)

XPC_CLASS_HEADER(uuid);

- (char*)xpcDescription
{
	char* output = NULL;
	char asString[UUID_STRING_LENGTH + 1];
	uuid_unparse(self.bytes, asString);
	asprintf(&output, "<%s: %s>", xpc_class_name(self), asString);
	return output;
}

- (uint8_t*)bytes
{
	XPC_THIS_DECL(uuid);
	return this->value;
}

- (instancetype)initWithBytes: (const uint8_t*)bytes
{
	if (self = [super init]) {
		XPC_THIS_DECL(uuid);
		memcpy(this->value, bytes, sizeof(this->value));
	}
	return self;
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(uuid);
	return xpc_raw_data_hash(this->value, sizeof(this->value));
}

@end

@implementation XPC_CLASS(uuid) (XPCSerialization)

- (BOOL)serializable
{
	return YES;
}

- (NSUInteger)serializationLength
{
	return xpc_serial_padded_length(sizeof(xpc_serial_type_t)) + xpc_serial_padded_length(sizeof(uuid_t));
}

+ (instancetype)deserialize: (XPC_CLASS(deserializer)*)deserializer
{
	XPC_CLASS(uuid)* result = nil;
	xpc_serial_type_t type = XPC_SERIAL_TYPE_INVALID;
	const uint8_t* bytes = NULL;

	if (![deserializer readU32: &type]) {
		goto error_out;
	}
	if (type != XPC_SERIAL_TYPE_UUID) {
		goto error_out;
	}

	if (![deserializer consume: sizeof(uuid_t) region: (const void**)&bytes]) {
		goto error_out;
	}

	result = [[[self class] alloc] initWithBytes: bytes];

	return result;

error_out:
	if (result != nil) {
		[result release];
	}
	return nil;
}

- (BOOL)serialize: (XPC_CLASS(serializer)*)serializer
{
	XPC_THIS_DECL(uuid);

	if (![serializer writeU32: XPC_SERIAL_TYPE_UUID]) {
		goto error_out;
	}

	if (![serializer write: this->value length: sizeof(this->value)]) {
		goto error_out;
	}

	return YES;

error_out:
	return NO;
}

@end

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_uuid_create(const uuid_t uuid) {
	return [[XPC_CLASS(uuid) alloc] initWithBytes: uuid];
};

XPC_EXPORT
const uint8_t* xpc_uuid_get_bytes(xpc_object_t xuuid) {
	TO_OBJC_CHECKED(uuid, xuuid, uuid) {
		return uuid.bytes;
	}
	return NULL;
};
