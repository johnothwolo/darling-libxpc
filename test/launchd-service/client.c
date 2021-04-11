#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <xpc/endpoint.h>
#include <stdio.h>

#include "service.h"

#define SECONDS_TO_WAIT 1

#define client_log(format, ...) printf("client connection: " format "\n", ## __VA_ARGS__)
#define client_error(format, ...) fprintf(stderr, "client connection: " format "\n", ## __VA_ARGS__)
#define reply_log(format, ...) printf("reply handler: " format "\n", ## __VA_ARGS__)
#define reply_error(format, ...) fprintf(stderr, "reply handler: " format "\n", ## __VA_ARGS__)
#define anon_client_log(format, ...) printf("anonymous client handler: " format "\n", ## __VA_ARGS__)
#define anon_client_error(format, ...) fprintf(stderr, "anonymous client handler: " format "\n", ## __VA_ARGS__)

#define server_peer_log(format, ...) printf("anonymous server peer connection: " format "\n", ## __VA_ARGS__)
#define server_peer_error(format, ...) fprintf(stderr, "anonymous server peer connection: " format "\n", ## __VA_ARGS__)
#define server_log(format, ...) printf("anonymous server connection: " format "\n", ## __VA_ARGS__)
#define server_error(format, ...) fprintf(stderr, "anonymous server connection: " format "\n", ## __VA_ARGS__)

#include "server_common.h"
#include "client_common.h"

dispatch_semaphore_t waiter;

static void reply_handler(bool anonymous, test_service_message_type_t outgoing_message_type, xpc_object_t object) {
	xpc_type_t obj_type = xpc_get_type(object);
	if (obj_type == (xpc_type_t)XPC_TYPE_DICTIONARY) {
		test_service_message_type_t incoming_message_type = xpc_dictionary_get_uint64(object, MESSAGE_TYPE_KEY);

		switch (outgoing_message_type) {
			case test_service_message_type_hello: {
				if (incoming_message_type != test_service_message_type_hello) {
					reply_error("server replied to hello message with invalid message with type: %llu", incoming_message_type);
					return;
				}

				reply_log("server replied to hello message with: %s", xpc_dictionary_get_string(object, HELLO_KEY));
			} break;

			case test_service_message_type_echo: {
				if (incoming_message_type != test_service_message_type_echo) {
					reply_error("server replied to echo message with invalid message with type: %llu", incoming_message_type);
					return;
				}

				char* desc = xpc_copy_description(xpc_dictionary_get_value(object, ECHO_KEY));
				reply_log("server replied to echo message with: %s", desc);
				if (desc) {
					free(desc);
				}
			} break;

			case test_service_message_type_meet_a_new_friend: {
				xpc_connection_t new_conn = NULL;
				xpc_object_t message = NULL;

				if (incoming_message_type != test_service_message_type_meet_a_new_friend) {
					reply_error("server replied to meet-a-new-friend message with invalid message with type: %llu", incoming_message_type);
					dispatch_semaphore_signal(waiter);
					return;
				}

				new_conn = xpc_dictionary_create_connection(object, MEET_A_NEW_FRIEND_KEY);
				if (!new_conn) {
					reply_log("server didn't have a friend for us to meet :(");
					dispatch_semaphore_signal(waiter);
					return;
				}

				xpc_connection_set_event_handler(new_conn, ^(xpc_object_t peer_object) {
					xpc_type_t obj_type = xpc_get_type(object);
					if (obj_type == (xpc_type_t)XPC_TYPE_DICTIONARY) {
						test_service_message_type_t server_message_type = xpc_dictionary_get_uint64(object, MESSAGE_TYPE_KEY);
					} else if (obj_type == (xpc_type_t)XPC_TYPE_ERROR) {
						if (object == XPC_ERROR_CONNECTION_INVALID) {
							anon_client_log("connection got cancelled\n");
						} else if (object == XPC_ERROR_CONNECTION_INTERRUPTED) {
							anon_client_log("connection got interrupted\n");
						} else {
							anon_client_error("received unexpected error: %s\n", xpc_copy_description(object));
							exit(1);
						}
					} else {
						anon_client_error("received non-dictionary, non-error object in event handler: %s\n", xpc_copy_description(object));
						exit(1);
					}
				});

				xpc_connection_resume(new_conn);

				message = xpc_dictionary_create(NULL, NULL, 0);
				xpc_dictionary_set_uint64(message, MESSAGE_TYPE_KEY, test_service_message_type_hello);
				xpc_dictionary_set_string(message, HELLO_KEY, "Hello from an anonymous client!");
				xpc_connection_send_message_with_reply(new_conn, message, NULL, ^(xpc_object_t reply) {
					reply_handler(true, test_service_message_type_hello, reply);
					dispatch_semaphore_signal(waiter);
				});
			} break;

			default: {
				reply_error("user requested unknown message type: %llu", outgoing_message_type);
			} break;
		}
	} else if (obj_type == (xpc_type_t)XPC_TYPE_ERROR) {
		if (object == XPC_ERROR_CONNECTION_INVALID) {
			reply_log("connection got cancelled\n");
		} else if (object == XPC_ERROR_CONNECTION_INTERRUPTED) {
			reply_log("connection got interrupted\n");
		} else {
			reply_error("received unexpected error: %s\n", xpc_copy_description(object));
			exit(1);
		}
	} else {
		reply_error("received non-dictionary, non-error object in reply handler: %s\n", xpc_copy_description(object));
		abort();
	}
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
			xpc_dictionary_set_string(reply, HELLO_KEY, "Hello from the anonymous server (as a reply)!");
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
			server_peer_error("received friendship invitation from client, but this is invalid");
		} break;

		case test_service_message_type_meet_a_new_friend: {
			server_peer_error("client wants to meet a new friend, but this is invalid");
		} break;

		default: {
			server_peer_error("received unknown message type: %llu", message_type);
		} break;
	}
};

static void connection_died(xpc_connection_t connection) {};

static xpc_endpoint_t setup_anonymous_server(void) {
	xpc_connection_t anon_server = xpc_connection_create(NULL, NULL);

	xpc_connection_set_event_handler(anon_server, ^(xpc_object_t object) {
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

	xpc_connection_resume(anon_server);

	return xpc_endpoint_create(anon_server);
};

int main(int argc, char** argv) {
	test_service_message_type_t message_type = determine_message_type(argc, argv);
	xpc_connection_t client = xpc_connection_create(TEST_SERVICE_NAME, NULL);
	xpc_object_t message = NULL;
	bool synchronous_reply = false;
	void (^send_with_reply)(void) = NULL;
	waiter = dispatch_semaphore_create(0);

	if (argc > 2) {
		synchronous_reply = true;
	}

	xpc_connection_set_event_handler(client, ^(xpc_object_t object) {
		xpc_type_t obj_type = xpc_get_type(object);
		if (obj_type == (xpc_type_t)XPC_TYPE_DICTIONARY) {
			test_service_message_type_t server_message_type = xpc_dictionary_get_uint64(object, MESSAGE_TYPE_KEY);
			switch (server_message_type) {
				case test_service_message_type_poke: {
					client_log("received poke from server");
				} break;

				case test_service_message_type_hello: {
					xpc_object_t reply = NULL;
					client_log("received hello message from server: %s", xpc_dictionary_get_string(object, HELLO_KEY));
					reply = xpc_dictionary_create_reply(object);
					xpc_dictionary_set_uint64(reply, MESSAGE_TYPE_KEY, test_service_message_type_hello);
					xpc_dictionary_set_string(reply, HELLO_KEY, "Hello from the client (as a reply)!");
					xpc_connection_send_message(xpc_dictionary_get_remote_connection(object), reply);
				} break;

				case test_service_message_type_echo: {
					xpc_object_t reply = NULL;
					xpc_object_t echo_item = xpc_dictionary_get_value(object, ECHO_KEY);
					char* desc = xpc_copy_description(echo_item);
					client_log("received echo request from server for dictionary item: %s", desc);
					if (desc) {
						free(desc);
					}
					reply = xpc_dictionary_create_reply(object);
					xpc_dictionary_set_uint64(reply, MESSAGE_TYPE_KEY, test_service_message_type_echo);
					xpc_dictionary_set_value(reply, ECHO_KEY, echo_item ? echo_item : xpc_null_create());
					xpc_connection_send_message(xpc_dictionary_get_remote_connection(object), reply);
				} break;

				case test_service_message_type_friendship_invitation: {
					client_error("received friendship invitation from server, but this is invalid");
				} break;

				case test_service_message_type_meet_a_new_friend: {
					client_error("server wants to meet a new friend, but this is invalid");
				} break;

				default: {
					client_error("server sent message with unknown type: %llu", server_message_type);
				} break;
			}
		} else if (obj_type == (xpc_type_t)XPC_TYPE_ERROR) {
			if (object == XPC_ERROR_CONNECTION_INVALID) {
				client_log("connection got cancelled\n");
			} else if (object == XPC_ERROR_CONNECTION_INTERRUPTED) {
				client_log("connection got interrupted\n");
			} else {
				client_error("received unexpected error: %s\n", xpc_copy_description(object));
				exit(1);
			}
		} else {
			client_error("received non-dictionary, non-error object in event handler: %s\n", xpc_copy_description(object));
			exit(1);
		}
	});

	xpc_connection_resume(client);

	message = xpc_dictionary_create(NULL, NULL, 0);

	xpc_dictionary_set_uint64(message, MESSAGE_TYPE_KEY, message_type);

	send_with_reply = ^{
		if (synchronous_reply) {
			reply_handler(false, message_type, xpc_connection_send_message_with_reply_sync(client, message));
			dispatch_semaphore_signal(waiter);
		} else {
			xpc_connection_send_message_with_reply(client, message, NULL, ^(xpc_object_t object) {
				reply_handler(false, message_type, object);
				dispatch_semaphore_signal(waiter);
			});
		}
	};

	switch (message_type) {
		case test_service_message_type_poke: {
			xpc_connection_send_message(client, message);

			client_log("poke message submitted, now we'll wait for a bit for the system to send it");

			// we gotta wait a little to ensure the connection is established and the message is sent
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, SECONDS_TO_WAIT * NSEC_PER_SEC), dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
				dispatch_semaphore_signal(waiter);
			});
		} break;

		case test_service_message_type_hello: {
			xpc_dictionary_set_string(message, HELLO_KEY, "Hello from the client!");
			send_with_reply();
		} break;

		case test_service_message_type_echo: {
			xpc_dictionary_set_string(message, ECHO_KEY, "ECHO... Echo... echo.. *whipsers* echo...");
			send_with_reply();
		} break;

		case test_service_message_type_friendship_invitation: {
			xpc_endpoint_t endpoint = setup_anonymous_server();
			xpc_dictionary_set_value(message, FRIENDSHIP_INVITATION_KEY, endpoint);
			xpc_connection_send_message(client, message);
			dispatch_main(); // stay alive forever because we're now a server
		} break;

		case test_service_message_type_meet_a_new_friend: {
			send_with_reply();
			dispatch_semaphore_wait(waiter, DISPATCH_TIME_FOREVER); // wait again because we need to wait for the anonymous server to respond
		} break;

		default: {
			client_error("user requested unknown message type: %llu", message_type);
			exit(1);
		} break;
	}

	dispatch_semaphore_wait(waiter, DISPATCH_TIME_FOREVER);

	return 0;
};
