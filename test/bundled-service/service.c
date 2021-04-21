#include <xpc/xpc.h>
#include <xpc/connection.h>

static void connection_handler(xpc_connection_t connection) {
	printf("Got a new connection: %p\n", connection);

	xpc_connection_set_event_handler(connection, ^(xpc_object_t object) {
		char* desc = xpc_copy_description(object);
		xpc_type_t type = xpc_get_type(object);
		printf("Recevied %s from connection %p: %s\n", (type == (xpc_type_t)XPC_TYPE_DICTIONARY) ? "message" : ((type == (xpc_type_t)XPC_TYPE_ERROR) ? "error" : "unexpected object"), connection, desc);
		free(desc);
	});
	xpc_connection_resume(connection);
};

int main(int argc, char** argv) {
	xpc_main(connection_handler);
	return 0;
};
