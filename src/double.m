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

#import <xpc/objects/double.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/serialization.h>

#include <math.h>

XPC_WRAPPER_CLASS_IMPL(double, double, "%f");

// i don't like serializing doubles to raw bytes,
// (because that assumes the peer uses the same binary represenation for doubles, which they might not)
// but this is the way Apple does it, so it has to be the way we do it

@implementation XPC_CLASS(double) (XPCSerialization)

- (BOOL)serializable
{
	return YES;
}

- (NSUInteger)serializationLength
{
	return xpc_serial_padded_length(sizeof(xpc_serial_type_t)) + xpc_serial_padded_length(sizeof(double));
}

+ (instancetype)deserialize: (XPC_CLASS(deserializer)*)deserializer
{
	XPC_CLASS(double)* result = nil;
	xpc_serial_type_t type = XPC_SERIAL_TYPE_INVALID;
	const double* data = NULL;

	if (![deserializer readU32: &type]) {
		goto error_out;
	}
	if (type != XPC_SERIAL_TYPE_DOUBLE) {
		goto error_out;
	}

	if (![deserializer consume: sizeof(double) region: (const void**)&data]) {
		goto error_out;
	}

	result = [[[self class] alloc] initWithValue: *data];

	return result;

error_out:
	if (result != nil) {
		[result release];
	}
	return nil;
}

- (BOOL)serialize: (XPC_CLASS(serializer)*)serializer
{
	XPC_THIS_DECL(double);
	double* data = NULL;

	if (![serializer writeU32: XPC_SERIAL_TYPE_DOUBLE]) {
		goto error_out;
	}

	if (![serializer reserve: sizeof(double) region: (void**)&data]) {
		goto error_out;
	}

	*data = this->value;

	return YES;

error_out:
	return NO;
}

@end

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_double_create(double value) {
	return [[XPC_CLASS(double) alloc] initWithValue: value];
};

XPC_EXPORT
double xpc_double_get_value(xpc_object_t xdouble) {
	TO_OBJC_CHECKED(double, xdouble, doubleObj) {
		return doubleObj.value;
	}
	return NAN;
};

//
// private C API
//

XPC_EXPORT
void _xpc_double_set_value(xpc_object_t xdouble, double value) {
	TO_OBJC_CHECKED(double, xdouble, doubleObj) {
		doubleObj.value = value;
	}
};
