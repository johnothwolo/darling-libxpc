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

#import <xpc/objects/error.h>
#import <xpc/objects/string.h>
#import <xpc/xpc.h>
#import <xpc/util.h>
#import <string.h>

XPC_CLASS_SYMBOL_DECL(error);

#define XPCErrorDescriptionKey "XPCErrorDescription"
const char* const _xpc_error_key_description = XPCErrorDescriptionKey;

// workaround like in `bool.m`
struct _xpc_dictionary_s {
	struct xpc_error_s the_real_thing;
};

#define XPC_ERROR_DEFINITION(_name, _description) \
	struct xpc_string_s _xpc_error_ ## _name ## _entry_string = { \
		.base = { \
			XPC_GLOBAL_OBJECT_HEADER(string), \
		}, \
		.byteLength = NSUIntegerMax, \
		.string = _description, \
		.freeWhenDone = false \
	}; \
	extern const struct _xpc_dictionary_s _xpc_error_ ## _name; \
	struct xpc_dictionary_entry_s _xpc_error_ ## _name ## _entry = { \
		.link = { \
			.le_next = NULL, \
			.le_prev = (struct xpc_dictionary_entry_s**)&LIST_FIRST(&_xpc_error_ ## _name.the_real_thing.base.head), \
		}, \
		.object = XPC_CAST(string, &_xpc_error_ ## _name ## _entry_string), \
		.name = XPCErrorDescriptionKey, \
	}; \
	XPC_EXPORT const struct _xpc_dictionary_s _xpc_error_ ## _name = { \
		.the_real_thing = { \
			.base = { \
				.base = { \
					XPC_GLOBAL_OBJECT_HEADER(error), \
				}, \
				.size = 1, \
				.head = { \
					.lh_first = &_xpc_error_ ## _name ## _entry, \
				}, \
			}, \
		}, \
	};

XPC_ERROR_DEFINITION(connection_interrupted, "Connection interrupted");
XPC_ERROR_DEFINITION(connection_invalid, "Connection invalid");
XPC_ERROR_DEFINITION(termination_imminent, "Termination imminent");

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(error)

XPC_CLASS_HEADER(error);

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s: %s>", xpc_class_name(self), [self stringForKey: XPCErrorDescriptionKey].CString);
	return output;
}

@end

//
// C API
//

char *_xpc_errors[] = {
    "Malformed bundle",
    "Invalid path",
    "Invalid property list",
    "Invalid or missing service identifier",
    "Invalid or missing Program/ProgramArguments",
    "Could not find specified domain",
    "Could not find specified service",
    "The specified username does not exist",
    "The specified group does not exist",
    "Routine not yet implemented",
    "(n/a)",
    "Bad response from server",
    "Service is disabled",
    "Bad subsystem destination for request",
    "Path not searched for services",
    "Path had bad ownership/permissions",
    "Path is whitelisted for domain",
    "Domain is tearing down",
    "Domain does not support specified action",
    "Request type is no longer supported",
    "The specified service did not ship with the operating system",
    "The specified path is not a bundle",
    "The service was superseded by a later version",
    "The system encountered a condition where behavior was undefined",
    "Out of order requests",
    "Request for stale data",
    "Multiple errors were returned; see stderr",
    "Service cannot load in requested session",
    "Process is not managed",
    "Action not allowed on singleton service",
    "Service does not support the specified actoin",
    "Service cannot be loaded on this hardware",
    "Service cannot presently execute",
    "Service name is reserved or invalid",
    "Reentrancy avoided",
    "Operation only supported on development build",
    "Requested entry was cached",
    "Requestor lacks required entitlement",
    "Endpoint is hidden",
    "Domain is in on-demand-only mode",
    "The specified service did not ship in the requestor",
    "The specified service path was not in the service cache",
    "Could not find a bundle of the given identifier through LaunchServices",
    "Operation not permitted while System Integrity Protection is engaged",
    "A complete hack",
    "Service cannot load in current boot environment",
    "Completely unexpected error",
    "Requestor is not a platform binary",
    "Refusing to execute/trust quarantined program/file",
    "Domain creation with that UID is not allowed anymore",
    "System service is not in system service whitelist",
    "Service cannot be loaded on current os variant",
    "Unknown error",
};

XPC_EXPORT
const char* xpc_strerror(int error) {
    char *result;
    
    if ( (unsigned)(error - 107) > 52 )
        result = strerror(error);
    else
        result = _xpc_errors[error - 106];
    return result;
};
