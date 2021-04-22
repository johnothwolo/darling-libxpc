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

#import <xpc/xpc.h>
#import <xpc/util.h>

// actually, the event publisher is it's own class
typedef xpc_object_t xpc_event_publisher_t;

XPC_EXPORT const char* const _xpc_event_key_name = "XPCEventName";
XPC_EXPORT const char* const _xpc_event_key_stream_name = "XPCEventStreamName";

//
// private C API
//

XPC_EXPORT
void xpc_event_publisher_activate(xpc_event_publisher_t xpub) {
	xpc_stub();
};

XPC_EXPORT
xpc_object_t xpc_event_publisher_create(const char* name, void* some_parameter) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
int xpc_event_publisher_fire(xpc_event_publisher_t xpub, uint64_t token, xpc_object_t details) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int xpc_event_publisher_fire_noboost(xpc_event_publisher_t xpub, uint64_t token, xpc_object_t details) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int xpc_event_publisher_fire_with_reply() {
	// i didn't feel like determining the parameters for this one
	// should be similar to `xpc_event_publisher_fire`
	xpc_stub();
	return -1;
};

XPC_EXPORT
xpc_object_t xpc_event_publisher_fire_with_reply_sync() {
	// should be similar to `xpc_event_publisher_fire_with_reply`
	xpc_stub();
	return NULL;
};

XPC_EXPORT
au_asid_t xpc_event_publisher_get_subscriber_asid(xpc_event_publisher_t xpub, uint64_t token) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
void xpc_event_publisher_set_error_handler(xpc_event_publisher_t xpub, void (^handler)()) {
	// no clue what the parameters for the handler are
	// (probably includes some sort of error parameter, but i have no clue what that is)
	xpc_stub();
};

XPC_EXPORT
void xpc_event_publisher_set_handler(xpc_event_publisher_t xpub, void (^handler)()) {
	// likewise, no clue what the parameters to the handler are
	xpc_stub();
};

XPC_EXPORT
void xpc_event_publisher_set_initial_load_completed_handler_4remoted(xpc_event_publisher_t xpub, void (^handler)()) {
	// once again, no clue about the handler parameters
	xpc_stub();
};

XPC_EXPORT
int xpc_event_publisher_set_subscriber_keepalive(xpc_event_publisher_t xpub, uint64_t token, bool state) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
int xpc_event_stream_check_in() {
	// not a stub
	// just returns 0
	return 0;
};

XPC_EXPORT
int xpc_event_stream_check_in2() {
	// not a stub
	// just returns 0
	return 0;
};

XPC_EXPORT
int xpc_get_event_name(const char* stream, uint64_t token, char* out_name) {
	xpc_stub();
	return -1;
};

XPC_EXPORT
xpc_object_t xpc_copy_event(const char* stream, const char* name) {
	// returns a dictionary
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_copy_event_entitlements(const char* stream, uint64_t token) {
	// returns a dictionary
	xpc_stub();
	return NULL;
};

XPC_EXPORT
void xpc_set_event(const char* stream, const char* name, xpc_object_t descriptor) {
	xpc_stub();
};

XPC_EXPORT
void xpc_set_event_state(const char* stream, uint64_t token, bool state) {
	xpc_stub();
};

XPC_EXPORT
void xpc_set_event_stream_handler(const char* connection_name, dispatch_queue_t queue, void (^handler)(xpc_object_t event)) {
	xpc_stub();
};

XPC_EXPORT
void xpc_set_event_with_flags(const char* stream, const char* name, xpc_object_t descriptor, uint64_t flags) {
	xpc_stub();
};
