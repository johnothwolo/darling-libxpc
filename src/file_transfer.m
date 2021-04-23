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

#import <xpc/objects/file_transfer.h>
#import <xpc/xpc.h>
#import <xpc/util.h>

XPC_CLASS_SYMBOL_DECL(file_transfer);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(file_transfer)

XPC_CLASS_HEADER(file_transfer);

@end

//
// private C API
//

XPC_EXPORT
void xpc_file_transfer_cancel(xpc_file_transfer_t xtransfer) {
	// actually just crashes
	xpc_stub();
};

XPC_EXPORT
dispatch_io_t xpc_file_transfer_copy_io(xpc_file_transfer_t xtransfer) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_file_transfer_t xpc_file_transfer_create_with_fd(int fd, void (^completionCallback)(void)) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_file_transfer_t xpc_file_transfer_create_with_path(const char* path, void (^completionCallback)(void)) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
uint64_t xpc_file_transfer_get_size(xpc_file_transfer_t xtransfer) {
	// return type is either `uint64_t` or `size_t`
	xpc_stub();
	return 0;
};

XPC_EXPORT
uint64_t xpc_file_transfer_get_transfer_id(xpc_file_transfer_t xtransfer) {
	xpc_stub();
	return 0;
};

XPC_EXPORT
void xpc_file_transfer_send_finished(xpc_file_transfer_t xtransfer, bool succeeded) {
	// unsure about the second parameter (both the return type and the purpose)
	xpc_stub();
};

XPC_EXPORT
void xpc_file_transfer_set_transport_writing_callbacks(xpc_file_transfer_t xtransfer, void (^writeCallback)(void), void (^sendCallback)(void)) {
	// parameters 2 & 3 are complete guesses;
	// the only thing i know about them is that they're blocks
	xpc_stub();
};

XPC_EXPORT
void xpc_file_transfer_write_finished(xpc_file_transfer_t xtransfer, bool succeeded) {
	// i'm guessing this function is like `xpc_file_transfer_send_finished`
	xpc_stub();
};

XPC_EXPORT
void* xpc_file_transfer_write_to_fd(xpc_file_transfer_t xtransfer, int fd, void (^someCallback)(void)) {
	// no clue what the return type is
	// also unsure about parameter 3's type
	xpc_stub();
	return NULL;
};

XPC_EXPORT
void* xpc_file_transfer_write_to_path(xpc_file_transfer_t xtransfer, const char* path, void (^someCallback)(void)) {
	// same issues as `xpc_file_transfer_write_to_fd`
	xpc_stub();
	return NULL;
};
