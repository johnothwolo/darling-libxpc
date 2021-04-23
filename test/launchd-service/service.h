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

#ifndef _XPC_TEST_LAUNCHD_SERVICE_SERVICE_H_
#define _XPC_TEST_LAUNCHD_SERVICE_SERVICE_H_

#include <stdint.h>

#define TEST_SERVICE_NAME "org.darlinghq.libxpc.test-service"

enum test_service_message_type {
	// an invalid message type
	test_service_message_type_invalid,

	// simple one-way message from either the client or the server with no content (besides the message type) and no expectation of a reply
	test_service_message_type_poke,

	// say hello to server and have it say hello back
	test_service_message_type_hello,

	// have the server echo anything you say
	test_service_message_type_echo,

	// tell the server you'd like to meet some friends
	test_service_message_type_friendship_invitation,

	// have the server introduce you to a client that's looking for friends
	test_service_message_type_meet_a_new_friend,
};

typedef uint64_t test_service_message_type_t;

#define MESSAGE_TYPE_KEY "message-type"

#define HELLO_KEY "hello"

#define ECHO_KEY "echo"

#define FRIENDSHIP_INVITATION_KEY "friendship-invitation"

#define MEET_A_NEW_FRIEND_KEY "meet-a-new-friend"

#endif // _XPC_TEST_LAUNCHD_SERVICE_SERVICE_H_
