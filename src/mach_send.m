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

#import <xpc/objects/mach_send.h>
#import <xpc/xpc.h>
#import <xpc/util.h>
#import <xpc/serialization.h>

XPC_CLASS_SYMBOL_DECL(mach_send);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(mach_send)

XPC_CLASS_HEADER(mach_send);

- (void)dealloc
{
	XPC_THIS_DECL(mach_send);
	xpc_mach_port_release_send_any(this->port);
	[super dealloc];
}

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s: port = %d>", xpc_class_name(self), self.port);
	return output;
}

- (mach_port_t)port
{
	XPC_THIS_DECL(mach_send);
	return this->port;
}

- (instancetype)initWithPort: (mach_port_t)port
{
	return [self initWithPort: port disposition: MACH_MSG_TYPE_COPY_SEND];
}

- (instancetype)initWithPortNoCopy: (mach_port_t)port
{
	return [self initWithPort: port disposition: MACH_MSG_TYPE_MOVE_SEND];
}

- (instancetype)initWithPort: (mach_port_t)port disposition: (mach_msg_type_name_t)disposition
{
	if (self = [super init]) {
		XPC_THIS_DECL(mach_send);
		if (disposition == MACH_MSG_TYPE_COPY_SEND) {
			if (xpc_mach_port_retain_send(port) != KERN_SUCCESS) {
				[self release];
				return nil;
			}
		} else if (disposition == MACH_MSG_TYPE_MAKE_SEND) {
			if (xpc_mach_port_make_send(port) != KERN_SUCCESS) {
				[self release];
				return nil;
			}
		}
		this->port = port;
	}
	return self;
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(mach_send);
	return (NSUInteger)this->port;
}

- (instancetype)copy
{
	XPC_THIS_DECL(mach_send);
	return [[XPC_CLASS(mach_send) alloc] initWithPort: this->port];
}

- (mach_port_t)extractPort
{
	XPC_THIS_DECL(mach_send);
	mach_port_t port = this->port;
	if (port == MACH_PORT_DEAD) {
		xpc_abort("attempt to extract port from a mach_send object more than once");
	}
	this->port = MACH_PORT_DEAD;
	return port;
}

- (mach_port_t)copyPort
{
	XPC_THIS_DECL(mach_send);
	if (xpc_mach_port_retain_send(this->port) != KERN_SUCCESS) {
		return MACH_PORT_NULL;
	}
	return this->port;
}

@end

@implementation XPC_CLASS(mach_send) (XPCSerialization)

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
	XPC_CLASS(mach_send)* result = nil;
	xpc_serial_type_t type = XPC_SERIAL_TYPE_INVALID;
	mach_port_t port = MACH_PORT_NULL;

	if (![deserializer readU32: &type]) {
		goto error_out;
	}
	if (type != XPC_SERIAL_TYPE_MACH_SEND) {
		goto error_out;
	}

	if (![deserializer readPort: &port type: MACH_MSG_TYPE_PORT_SEND]) {
		goto error_out;
	}

	result = [[[self class] alloc] initWithPortNoCopy: port];

	return result;

error_out:
	if (result != nil) {
		[result release];
	}
	return nil;
}

- (BOOL)serialize: (XPC_CLASS(serializer)*)serializer
{
	XPC_THIS_DECL(mach_send);

	if (![serializer writeU32: XPC_SERIAL_TYPE_MACH_SEND]) {
		goto error_out;
	}

	if (![serializer writePort: this->port type: MACH_MSG_TYPE_COPY_SEND]) {
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
mach_port_t xpc_mach_send_copy_right(xpc_object_t xsend) {
	TO_OBJC_CHECKED(mach_send, xsend, send) {
		return [send copyPort];
	}
	return MACH_PORT_NULL;
};

XPC_EXPORT
xpc_object_t xpc_mach_send_create(mach_port_t send) {
	return [[XPC_CLASS(mach_send) alloc] initWithPort: send];
};

XPC_EXPORT
xpc_object_t xpc_mach_send_create_with_disposition(mach_port_t send, unsigned int disposition) {
	return [[XPC_CLASS(mach_send) alloc] initWithPort: send disposition: disposition];
};

XPC_EXPORT
mach_port_t xpc_mach_send_get_right(xpc_object_t xsend) {
	TO_OBJC_CHECKED(mach_send, xsend, send) {
		return send.port;
	}
	return MACH_PORT_NULL;
};

mach_port_t xpc_mach_send_extract_right(xpc_object_t xsend) {
	TO_OBJC_CHECKED(mach_send, xsend, send) {
		return [send extractPort];
	}
	return MACH_PORT_NULL;
};
