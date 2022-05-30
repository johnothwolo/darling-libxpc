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

#ifndef _XPC_OBJECTS_SERIALIZER_H_
#define _XPC_OBJECTS_SERIALIZER_H_

#import <xpc/objects/base.h>

#include <dispatch/dispatch.h>
#ifndef __DISPATCH_INDIRECT__
#define __DISPATCH_INDIRECT__ 1
#endif
#include <dispatch/mach_private.h>

#define MACH_MSG_SEND_DISPOSITION_FIRST MACH_MSG_TYPE_MOVE_RECEIVE
#define MACH_MSG_SEND_DISPOSITION_LAST  MACH_MSG_TYPE_MAKE_SEND_ONCE
#define MACH_MSG_SEND_DISPOSITION_COUNT (MACH_MSG_SEND_DISPOSITION_LAST - MACH_MSG_SEND_DISPOSITION_FIRST + 1)

XPC_CLASS_DECL(serializer);

typedef struct xpc_serial_port_array {
	mach_port_t* array;
	size_t length;
} xpc_serial_port_array_t;

struct xpc_serializer_s {
	struct xpc_object_s base;
	dispatch_mach_msg_t finalized_message;
	size_t length;
	size_t offset;
	void* buffer;
	xpc_serial_port_array_t port_arrays[MACH_MSG_SEND_DISPOSITION_COUNT];
};

@class XPC_CLASS(dictionary);

@interface XPC_CLASS_INTERFACE(serializer)

@property(readonly) NSUInteger offset;
@property(readonly) BOOL isFinalized;

/**
 * Creates a new, autoreleased serializer.
 */
+ (instancetype)serializer;

/**
 * Initializes the serializer without automatically writing in the XPC serial message header (i.e. XPC magic and version).
 */
- (instancetype)initWithoutHeader;

/**
 * Finalizes the serializer and packs all the content written to it into a Mach message.
 *
 * After this method is called, you can no longer write new content.
 * Subsequent calls to this method simply return the same message.
 *
 * The finalized message is retained by the serializer and remains valid for as long as the serializer does.
 *
 * The returned message is not automatically retained. Therefore, if you want the message to live past the serializer,
 * make sure to retain it yourself.
 */
- (dispatch_mach_msg_t)finalizeWithRemotePort: (mach_port_t)remotePort localPort: (mach_port_t)localPort asReply: (BOOL)asReply expectingReply: (BOOL)expectingReply messageID: (uint32_t)messageID;

/**
 * Like the other `finalizeWithRemotePort:...`, but automatically determines the correct XPC message ID.
 *
 * @see finalizeWithRemotePort:localPort:asReply:expectingReply:messageID:
 */
- (dispatch_mach_msg_t)finalizeWithRemotePort: (mach_port_t)remotePort localPort: (mach_port_t)localPort asReply: (BOOL)asReply expectingReply: (BOOL)expectingReply;

/**
 * Determines whether the internal buffer would have to be expanded to append content of the given size.
 */
- (BOOL)needsToResizeToWrite: (NSUInteger)extraSize;

/**
 * Determines if the internal buffer has enough extra space to append content of the given size,
 * and if not, automatically resizes the internal buffer.
 *
 * This method only returns `NO` when the internal buffer failed to resize.
 */
- (BOOL)ensure: (NSUInteger)extraSize;

// NOTE: all writes to the internal buffer are subject to padding,
//       so the number of bytes passed in might not be the same as number of bytes actually written.

- (BOOL)write: (const void*)data length: (NSUInteger)length;
- (BOOL)reserve: (NSUInteger)length region: (void**)region;
- (BOOL)writeString: (const char*)string;
- (BOOL)writeU32: (uint32_t)value;
- (BOOL)writeU64: (uint64_t)value;
- (BOOL)writePort: (mach_port_t)port type: (mach_msg_type_name_t)type;
- (BOOL)writeObject: (XPC_CLASS(object)*)object;

@end

#endif // _XPC_OBJECTS_SERIALIZER_H_
