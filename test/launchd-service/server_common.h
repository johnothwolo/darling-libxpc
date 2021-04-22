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

#ifndef _XPC_TEST_LAUNCHD_SERVICE_SERVER_COMMON_H_
#define _XPC_TEST_LAUNCHD_SERVICE_SERVER_COMMON_H_

#include <xpc/xpc.h>
#include <xpc/connection.h>

#ifndef server_peer_log
	#define server_peer_log(...)
#endif
#ifndef server_peer_error
	#define server_peer_error(...)
#endif

#ifndef server_log
	#define server_log(...)
#endif
#ifndef server_error
	#define server_error(...)
#endif

static void handle_server_peer_message(xpc_object_t message);
static void connection_died(xpc_connection_t connection);

static void handle_server_peer_error(xpc_connection_t connection, xpc_object_t error) {
	if (error == XPC_ERROR_CONNECTION_INVALID) {
		server_peer_log("client died (or the parent server did)\n");
		connection_died(connection);
	} else if (error == XPC_ERROR_TERMINATION_IMMINENT) {
		server_peer_log("someone wants to kill us\n");
	} else {
		server_peer_error("received unexpected error: %s", xpc_copy_description(error));
		exit(1);
	}
};

static void handle_new_connection(xpc_connection_t connection) {
	server_peer_log("got new client with pid=%d, euid=%d, and egid=%d", xpc_connection_get_pid(connection), xpc_connection_get_euid(connection), xpc_connection_get_egid(connection));
	xpc_connection_set_event_handler(connection, ^(xpc_object_t object) {
		xpc_type_t obj_type = xpc_get_type(object);
		if (obj_type == (xpc_type_t)XPC_TYPE_DICTIONARY) {
			handle_server_peer_message(object);
		} else if (obj_type == (xpc_type_t)XPC_TYPE_ERROR) {
			handle_server_peer_error(connection, object);
		} else {
			server_peer_error("received non-connection, non-error object in event handler: %s\n", xpc_copy_description(object));
			exit(1);
		}
	});
	xpc_connection_resume(connection);
};

static void handle_server_error(xpc_object_t error) {
	if (error == XPC_ERROR_CONNECTION_INVALID) {
		server_log("server died (or got cancelled)\n");
	} else if (error == XPC_ERROR_TERMINATION_IMMINENT) {
		server_log("someone wants to kill us\n");
	} else {
		server_error("received unexpected error: %s\n", xpc_copy_description(error));
		exit(1);
	}
};

#endif // _XPC_TEST_LAUNCHD_SERVICE_SERVER_COMMON_H_
