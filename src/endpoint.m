#import <xpc/objects/endpoint.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/serialization.h>

XPC_CLASS_SYMBOL_DECL(endpoint);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(endpoint)

XPC_CLASS_HEADER(endpoint);

- (mach_port_t)port
{
	XPC_THIS_DECL(endpoint);
	return this->port;
}

- (void)setPort: (mach_port_t)port
{
	XPC_THIS_DECL(endpoint);
	xpc_mach_port_retain_send(port);
	xpc_mach_port_release_send(this->port);
	this->port = port;
}

- (void)dealloc
{
	self.port = MACH_PORT_NULL;
	[super dealloc];
}

- (instancetype)initWithConnection: (XPC_CLASS(connection)*)connection
{
	return [self initWithPort: connection.sendPort];
}

- (instancetype)initWithPort: (mach_port_t)port
{
	xpc_mach_port_retain_send(port);
	return [self initWithPortNoCopy: port];
}

- (instancetype)initWithPortNoCopy: (mach_port_t)port
{
	if (self = [super init]) {
		XPC_THIS_DECL(endpoint);
		this->port = port;
	}
	return self;
}

- (int)compare: (XPC_CLASS(endpoint)*)rhs
{
	int diff = self.port - rhs.port;
	return (diff == 0) ? 0 : ((diff < 0) ? -1 : 1);
}

@end

@implementation XPC_CLASS(endpoint) (XPCSerialization)

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
	XPC_CLASS(endpoint)* result = nil;
	xpc_serial_type_t type = XPC_SERIAL_TYPE_INVALID;
	mach_port_t port = MACH_PORT_NULL;

	if (![deserializer readU32: &type]) {
		goto error_out;
	}
	if (type != XPC_SERIAL_TYPE_ENDPOINT) {
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
	XPC_THIS_DECL(endpoint);

	if (![serializer writeU32: XPC_SERIAL_TYPE_ENDPOINT]) {
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
// C API
//

XPC_EXPORT
xpc_endpoint_t xpc_endpoint_create(xpc_connection_t xconn) {
	TO_OBJC_CHECKED(connection, xconn, conn) {
		return [[XPC_CLASS(endpoint) alloc] initWithConnection: conn];
	}
	return NULL;
};

//
// private C API
//

XPC_EXPORT
int xpc_endpoint_compare(xpc_endpoint_t xlhs, xpc_endpoint_t xrhs) {
	TO_OBJC_CHECKED(endpoint, xlhs, lhs) {
		TO_OBJC_CHECKED(endpoint, xrhs, rhs) {
			return [lhs compare: rhs];
		}
	}

	return 0;
};

XPC_EXPORT
mach_port_t xpc_endpoint_copy_listener_port_4sim(xpc_endpoint_t xendpoint) {
	TO_OBJC_CHECKED(endpoint, xendpoint, endpoint) {
		mach_port_t port = endpoint.port;
		xpc_mach_port_retain_send(port);
		return port;
	}
	return MACH_PORT_NULL;
};

XPC_EXPORT
xpc_endpoint_t xpc_endpoint_create_bs_named(const char* name, uint64_t flags, uint8_t* out_type) {
	// parameter 3's purpose is a guess
	return NULL;
};

XPC_EXPORT
xpc_endpoint_t xpc_endpoint_create_mach_port_4sim(mach_port_t port) {
	return [[XPC_CLASS(endpoint) alloc] initWithPortNoCopy: port];
};
