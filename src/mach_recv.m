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

#import <xpc/objects/mach_recv.h>
#import <xpc/xpc.h>
#import <xpc/util.h>
#import <xpc/serialization.h>

XPC_CLASS_SYMBOL_DECL(mach_recv);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(mach_recv)

XPC_CLASS_HEADER(mach_recv);

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s: port = %d>", xpc_class_name(self), self.port);
	return output;
}

- (mach_port_t)port
{
	XPC_THIS_DECL(mach_recv);
	return this->port;
}

- (instancetype)initWithPort: (mach_port_t)port
{
	if (self = [super init]) {
		XPC_THIS_DECL(mach_recv);
		this->port = port;
	}
	return self;
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(mach_recv);
	return (NSUInteger)this->port;
}

- (instancetype)copy
{
	XPC_THIS_DECL(mach_recv);
	return [[XPC_CLASS(mach_recv) alloc] initWithPort: this->port];
}

- (mach_port_t)extractPort
{
	XPC_THIS_DECL(mach_recv);
	mach_port_t port = this->port;
	if (port == MACH_PORT_DEAD) {
		xpc_abort("attempt to extract port from a mach_recv object more than once");
	}
	this->port = MACH_PORT_DEAD;
	return port;
}

@end

@implementation XPC_CLASS(mach_recv) (XPCSerialization)

- (BOOL)serializable
{
	return YES;
}

- (NSUInteger)serializationLength
{
	return xpc_serial_padded_length(sizeof(xpc_serial_type_t));
}

+ (instancetype)deserialize: (XPC_CLASS(deserializer)*)deserializer
{
	XPC_CLASS(mach_recv)* result = nil;
	xpc_serial_type_t type = XPC_SERIAL_TYPE_INVALID;
	mach_port_t port = MACH_PORT_NULL;

	if (![deserializer readU32: &type]) {
		goto error_out;
	}
	if (type != XPC_SERIAL_TYPE_MACH_RECV) {
		goto error_out;
	}

	if (![deserializer readPort: &port type: MACH_MSG_TYPE_PORT_RECEIVE]) {
		goto error_out;
	}

	result = [[[self class] alloc] initWithPort: port];

	return result;

error_out:
	if (result != nil) {
		[result release];
	}
	return nil;
}

- (BOOL)serialize: (XPC_CLASS(serializer)*)serializer
{
	XPC_THIS_DECL(mach_recv);

	if (![serializer writeU32: XPC_SERIAL_TYPE_MACH_RECV]) {
		goto error_out;
	}

	if (![serializer writePort: this->port type: MACH_MSG_TYPE_MOVE_RECEIVE]) {
		goto error_out;
	}

	return YES;

error_out:
	return NO;
}

@end

//
// private C API
//

XPC_EXPORT
xpc_object_t xpc_mach_recv_create(mach_port_t recv) {
	return [[XPC_CLASS(mach_recv) alloc] initWithPort: recv];
};

XPC_EXPORT
mach_port_t xpc_mach_recv_extract_right(xpc_object_t xrecv) {
	TO_OBJC_CHECKED(mach_recv, xrecv, recv) {
		return [recv extractPort];
	}
	return MACH_PORT_NULL;
};
