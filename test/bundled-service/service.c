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
