#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <stdio.h>
#include <pthread/pthread.h>

#include <xpc/generic_array.h>

#include "service.h"
#include "../test-util.h"

#define server_peer_log(format, ...) printf("server peer connection: " format "\n", ## __VA_ARGS__)
#define server_peer_error(format, ...) fprintf(stderr, "server peer connection: " format "\n", ## __VA_ARGS__)
#define server_log(format, ...) printf("server connection: " format "\n", ## __VA_ARGS__)
#define server_error(format, ...) fprintf(stderr, "server connection: " format "\n", ## __VA_ARGS__)

#include "server_common.h"

XPC_GENARR_DECL(endpoint, xpc_endpoint_t, static);
XPC_GENARR_STRUCT(endpoint, xpc_endpoint_t);
XPC_GENARR_DEF(endpoint, xpc_endpoint_t, static);

XPC_GENARR_DECL(connection, xpc_connection_t, static);
XPC_GENARR_STRUCT(connection, xpc_connection_t);
XPC_GENARR_DEF(connection, xpc_connection_t, static);
XPC_GENARR_SEARCH_DECL(connection, xpc_connection_t, static);
XPC_GENARR_SEARCH_DEF(connection, xpc_connection_t, static);

static void endpoint_dtor(xpc_endpoint_t* endpoint) {
	xpc_release(*endpoint);
};

static pthread_rwlock_t endpoints_lock = PTHREAD_RWLOCK_INITIALIZER;
static xpc_genarr_connection_t endpoint_connections = XPC_GENARR_INITIALIZER(true, NULL);
static xpc_genarr_endpoint_t endpoints = XPC_GENARR_INITIALIZER(true, endpoint_dtor);

static void associate_endpoint(xpc_connection_t connection, xpc_endpoint_t endpoint) {
	size_t existing_index = 0;
	xpc_retain(endpoint);
	pthread_rwlock_wrlock(&endpoints_lock);
	existing_index = xpc_genarr_connection_find(&endpoint_connections, &connection);
	if (existing_index == SIZE_MAX) {
		xpc_genarr_connection_append(&endpoint_connections, &connection);
		xpc_genarr_endpoint_append(&endpoints, &endpoint);
	} else {
		xpc_genarr_endpoint_set(&endpoints, existing_index, &endpoint);
	}
	pthread_rwlock_unlock(&endpoints_lock);
};

static void connection_died(xpc_connection_t connection) {
	size_t existing_index = 0;
	pthread_rwlock_wrlock(&endpoints_lock);
	existing_index = xpc_genarr_connection_find(&endpoint_connections, &connection);
	if (existing_index != SIZE_MAX) {
		xpc_genarr_connection_remove(&endpoint_connections, existing_index);
		xpc_genarr_endpoint_remove(&endpoints, existing_index);
	}
	pthread_rwlock_unlock(&endpoints_lock);
};

static void handle_server_peer_message(xpc_object_t message) {
	test_service_message_type_t message_type = xpc_dictionary_get_uint64(message, MESSAGE_TYPE_KEY);

	switch (message_type) {
		case test_service_message_type_poke: {
			server_peer_log("received poke from client");
		} break;

		case test_service_message_type_hello: {
			xpc_object_t reply = NULL;
			server_peer_log("received hello message from client: %s", xpc_dictionary_get_string(message, HELLO_KEY));
			reply = xpc_dictionary_create_reply(message);
			xpc_dictionary_set_uint64(reply, MESSAGE_TYPE_KEY, test_service_message_type_hello);
			xpc_dictionary_set_string(reply, HELLO_KEY, "Hello from the server (as a reply)!");
			xpc_connection_send_message(xpc_dictionary_get_remote_connection(message), reply);
		} break;

		case test_service_message_type_echo: {
			xpc_object_t reply = NULL;
			xpc_object_t echo_item = xpc_dictionary_get_value(message, ECHO_KEY);
			char* desc = xpc_copy_description(echo_item);
			server_peer_log("received echo request from client for dictionary item: %s", desc);
			if (desc) {
				free(desc);
			}
			reply = xpc_dictionary_create_reply(message);
			xpc_dictionary_set_uint64(reply, MESSAGE_TYPE_KEY, test_service_message_type_echo);
			xpc_dictionary_set_value(reply, ECHO_KEY, echo_item ? echo_item : xpc_null_create());
			xpc_connection_send_message(xpc_dictionary_get_remote_connection(message), reply);
		} break;

		case test_service_message_type_friendship_invitation: {
			xpc_endpoint_t endpoint = xpc_dictionary_get_value(message, FRIENDSHIP_INVITATION_KEY);
			server_peer_log("received friendship invitation from client");
			if (!endpoint || xpc_get_type(endpoint) != (xpc_type_t)XPC_TYPE_ENDPOINT) {
				server_peer_error("friendship invitation was not an endpoint!");
				return;
			}
			associate_endpoint(xpc_dictionary_get_connection(message), endpoint);
		} break;

		case test_service_message_type_meet_a_new_friend: {
			xpc_object_t reply = NULL;
			size_t index = 0;
			size_t count = 0;
			server_peer_log("client wants to meet a new friend");
			reply = xpc_dictionary_create_reply(message);
			xpc_dictionary_set_uint64(reply, MESSAGE_TYPE_KEY, test_service_message_type_meet_a_new_friend);
			pthread_rwlock_rdlock(&endpoints_lock);
			count = xpc_genarr_endpoint_length(&endpoints);
			if (count == 0) {
				xpc_dictionary_set_value(reply, MEET_A_NEW_FRIEND_KEY, xpc_null_create());
			} else {
				xpc_endpoint_t endpoint = NULL;
				index = rand_index(count);
				xpc_genarr_endpoint_get(&endpoints, index, &endpoint);
				xpc_dictionary_set_value(reply, MEET_A_NEW_FRIEND_KEY, endpoint ? endpoint : xpc_null_create());
			}
			pthread_rwlock_unlock(&endpoints_lock);
			xpc_connection_send_message(xpc_dictionary_get_remote_connection(message), reply);
		} break;

		default: {
			server_peer_error("received unknown message type: %llu", message_type);
		} break;
	}
};

int main(int arc, char** argv) {
	xpc_connection_t server = xpc_connection_create_mach_service(TEST_SERVICE_NAME, NULL, XPC_CONNECTION_MACH_SERVICE_LISTENER);

	xpc_connection_set_event_handler(server, ^(xpc_object_t object) {
		xpc_type_t obj_type = xpc_get_type(object);
		if (obj_type == (xpc_type_t)XPC_TYPE_CONNECTION) {
			handle_new_connection(object);
		} else if (obj_type == (xpc_type_t)XPC_TYPE_ERROR) {
			handle_server_error(object);
		} else {
			server_error("received non-connection, non-error object in event handler: %s\n", xpc_copy_description(object));
			exit(1);
		}
	});

	xpc_connection_resume(server);

	dispatch_main();
	return 0;
};
