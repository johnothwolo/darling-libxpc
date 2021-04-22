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

XPC_EXPORT const char* XPC_COALITION_INFO_KEY_BUNDLE_IDENTIFIER = "bundle_identifier";
XPC_EXPORT const char* XPC_COALITION_INFO_KEY_CID = "cid";
XPC_EXPORT const char* XPC_COALITION_INFO_KEY_NAME = "name";
XPC_EXPORT const char* XPC_COALITION_INFO_KEY_RESOURCE_USAGE_BLOB = "resource-usage-blob";

//
// private C API
//

XPC_EXPORT
xpc_object_t xpc_coalition_copy_info(uint64_t cid) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
int xpc_coalition_history_pipe_async(int flags) {
	xpc_stub();
	return -1;
};
