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

#import <xpc/private/bundle.h>

#import <xpc/objects/bundle.h>
#import <xpc/objects/dictionary.h>
#import <xpc/objects/string.h>
#import <xpc/objects/connection.h>

#import <objc/runtime.h>

#import <Foundation/NSRunLoop.h>
#import <dlfcn.h>
#import <AppKit/NSApplication.h>
#import <crt_externs.h>

#define INFO_DICT_IDENTIFIER_KEY "CFBundleIdentifier"
#define INFO_DICT_PACKAGE_TYPE_KEY "CFBundlePackageType"

#define XPC_SERVICE_DICT_RUNLOOP_TYPE_KEY "RunLoopType"

#define XPC_PACKAGE_TYPE "XPC!"

OS_ENUM(xpc_service_runloop_type, uint8_t,
	xpc_service_runloop_type_invalid,
	xpc_service_runloop_type_dispatch,
	xpc_service_runloop_type_nsrunloop,
	xpc_service_runloop_type_nsapplicationmain,
	xpc_service_runloop_type_uiapplicationmain,
);

XPC_EXPORT
xpc_object_t _xpc_runtime_get_entitlements_data(void) {
	// returns a data object
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t _xpc_runtime_get_self_entitlements(void) {
	// returns a dictionary (parsed from the plist data from `_xpc_runtime_get_entitlements_data`)
	xpc_stub();
	return NULL;
};

XPC_EXPORT
bool _xpc_runtime_is_app_sandboxed(void) {
	xpc_stub();
	return false;
};

static xpc_service_runloop_type_t runloop_name_to_type(const char* name) {
	if (!name) {
		return xpc_service_runloop_type_dispatch;
	} else if (strcmp(name, "_UIApplicationMain") == 0) {
		return xpc_service_runloop_type_uiapplicationmain;
	} else if (strcmp(name, "_NSApplicationMain") == 0) {
		return xpc_service_runloop_type_nsapplicationmain;
	} else if (strcmp(name, "NSRunLoop") == 0 || strcmp(name, "_WebKit") == 0) {
		return xpc_service_runloop_type_nsrunloop;
	} else {
		return xpc_service_runloop_type_dispatch;
	}
};

XPC_EXPORT
void xpc_main(xpc_connection_handler_t handler) {
	@autoreleasepool {
		XPC_CLASS(bundle)* mainBundle = [XPC_CLASS(bundle) mainBundle];
		XPC_CLASS(string)* identifier = nil;
		xpc_service_runloop_type_t runloopType = xpc_service_runloop_type_invalid;
		XPC_CLASS(connection)* server = nil;

		if (!mainBundle) {
			xpc_abort("failed to retrieve main bundle information");
		}

		[mainBundle resolve];
		if (mainBundle.error != 0) {
			xpc_abort("failed to resolve main bundle information");
		}

		if (![[mainBundle.infoDictionary stringForKey: INFO_DICT_PACKAGE_TYPE_KEY] isEqualToString: XPC_PACKAGE_TYPE]) {
			xpc_abort("main bundle was not an XPC service bundle");
		}

		identifier = [mainBundle.infoDictionary stringForKey: INFO_DICT_IDENTIFIER_KEY];
		if (!identifier) {
			xpc_abort("failed to determine main bundle identifier");
		}

		server = [[XPC_CLASS(connection) alloc] initAsServerForService: identifier.CString queue: NULL];
		if (!server) {
			xpc_abort("failed to create server connection");
		}

		server.eventHandler = ^(xpc_object_t object) {
			xpc_type_t type = xpc_get_type(object);
			if (type == (xpc_type_t)XPC_TYPE_CONNECTION) {
				handler(XPC_CAST(connection, object));
			} else if (type == (xpc_type_t)XPC_TYPE_ERROR) {
				if (object == XPC_ERROR_TERMINATION_IMMINENT) {
					xpc_log(XPC_LOG_WARNING, "someone wants us to terminate");
				} else {
					xpc_abort("unexpected error receive in managed server event handler: %s", xpc_copy_description(object));
				}
			} else {
				xpc_abort("invalid object received in managed server event handler: %s", xpc_copy_description(object));
			}
		};

		// schedule the connection to be activated once the runloop is kicked off
		dispatch_async(dispatch_get_main_queue(), ^{
			[server activate];
		});

		runloopType = runloop_name_to_type([XPC_CAST(dictionary, xpc_bundle_get_xpcservice_dictionary(mainBundle)) stringForKey: XPC_SERVICE_DICT_RUNLOOP_TYPE_KEY].CString);

		switch (runloopType) {
			case xpc_service_runloop_type_dispatch: {
				dispatch_main();
			} break;

			case xpc_service_runloop_type_nsrunloop: {
				Class NSRunLoopClass = objc_getClass("NSRunLoop");
				if (!NSRunLoopClass) {
					xpc_abort("failed to load NSRunLoop class");
				}
				[[NSRunLoopClass currentRunLoop] run];
			} break;

			case xpc_service_runloop_type_nsapplicationmain: {
				void* appkit = dlopen("/System/Library/Frameworks/AppKit.framework/AppKit", RTLD_LAZY);
				if (!appkit) {
					xpc_abort("failed to load AppKit");
				}

				__typeof__(NSApplicationMain)* NSApplicationMain_ptr = dlsym(appkit, "NSApplicationMain");
				if (!NSApplicationMain_ptr) {
					xpc_abort("failed to load NSApplicationMain from AppKit");
				}

				NSApplicationMain_ptr(*_NSGetArgc(), (const char**)*_NSGetArgv());
			} break;

			case xpc_service_runloop_type_uiapplicationmain: {
				xpc_abort("UIApplicationMain runloop not implemented");
			} break;

			default: {
				xpc_abort("failed to determine runloop type");
			} break;
		}
	}

	xpc_abort("runloop returned");
};

XPC_EXPORT
void xpc_init_services(void) {
	xpc_stub();
};

XPC_EXPORT
void xpc_impersonate_user(void) {
	// not a stub
	// this function just does nothing
};
