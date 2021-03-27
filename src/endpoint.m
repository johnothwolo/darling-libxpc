#import <xpc/objects/endpoint.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(endpoint);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(endpoint)

XPC_CLASS_HEADER(endpoint);

- (instancetype)initWithConnection: (XPC_CLASS(connection)*)connection
{
	if (self = [super init]) {
		#warning TODO: -[XPC_CLASS(endpoint) initWithConnection:]
	}
	return self;
}

@end

//
// C API
//

XPC_EXPORT
xpc_endpoint_t xpc_endpoint_create(xpc_connection_t xconn) {
	return [[XPC_CLASS(endpoint) alloc] initWithConnection: XPC_CAST(connection, xconn)];
};

//
// private C API
//

XPC_EXPORT
int xpc_endpoint_compare(xpc_endpoint_t lhs, xpc_endpoint_t rhs) {
	return 0;
};

XPC_EXPORT
mach_port_t xpc_endpoint_copy_listener_port_4sim(xpc_endpoint_t xendpoint) {
	return MACH_PORT_NULL;
};

XPC_EXPORT
xpc_endpoint_t xpc_endpoint_create_bs_named(const char* name, uint64_t flags, uint8_t* out_type) {
	// parameter 3's purpose is a guess
	return NULL;
};

XPC_EXPORT
xpc_endpoint_t xpc_endpoint_create_mach_port_4sim(mach_port_t port) {
	return NULL;
};
