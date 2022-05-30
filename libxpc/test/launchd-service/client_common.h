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

#ifndef _XPC_TEST_LAUNCHD_SERVICE_CLIENT_COMMON_H_
#define _XPC_TEST_LAUNCHD_SERVICE_CLIENT_COMMON_H_

#include <stdlib.h>

#include "service.h"

#ifndef client_log
	#define client_log(...)
#endif

#ifndef client_error
	#define client_error(...)
#endif

static test_service_message_type_t determine_message_type(int argc, char** argv) {
	test_service_message_type_t message_type = test_service_message_type_invalid;

	if (argc > 1) {
		char first = argv[1][0];
		if (first == 'p' || first == 'P') {
			message_type = test_service_message_type_poke;
		} else if (first == 'h' || first == 'H') {
			message_type = test_service_message_type_hello;
		} else if (first == 'e' || first == 'E') {
			message_type = test_service_message_type_echo;
		} else if (first == 'f' || first == 'F') {
			message_type = test_service_message_type_friendship_invitation;
		} else if (first == 'm' || first == 'M') {
			message_type = test_service_message_type_meet_a_new_friend;
		} else {
			client_error("unparsable argument: %s", argv[1]);
			exit(1);
		}
	}

	return message_type;
};

#endif // _XPC_TEST_LAUNCHD_SERVICE_CLIENT_COMMON_H_
