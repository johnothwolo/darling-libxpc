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

#ifndef _XPC_OBJECTS_PIPE_H_
#define _XPC_OBJECTS_PIPE_H_

#import <xpc/objects/base.h>

#include <mach/mach.h>
#include <pthread/pthread.h>

XPC_IGNORE_DUPLICATE_PROTOCOL_PUSH;
XPC_CLASS_DECL(pipe);
XPC_IGNORE_DUPLICATE_PROTOCOL_POP;

typedef boolean_t (*xpc_pipe_mig_demux_f)(mach_msg_header_t* request, mach_msg_header_t* reply);

OS_ENUM(xpc_pipe_state, uint8_t,
	xpc_pipe_state_initial,
	xpc_pipe_state_active,
	xpc_pipe_state_broken,
);

struct xpc_pipe_s {
	struct xpc_object_s base;
	mach_port_t checkin_port;
	mach_port_t send_port;
	mach_port_t recv_port;
	pthread_rwlock_t state_lock;
	xpc_pipe_state_t state;
};

@class XPC_CLASS(dictionary);

@interface XPC_CLASS_INTERFACE(pipe)

/**
 * Whether the pipe is broken.
 *
 * @note The setter for this property is one-way: once it is set to `YES`, it cannot be unset.
 */
@property(assign) BOOL broken;

+ (int)receiveWithPort: (mach_port_t)port incomingMessage: (xpc_object_t*)incomingMessage flags: (uint64_t)flags;
+ (int)tryReceiveWithPort: (mach_port_t)port incomingMessage: (xpc_object_t*)incomingMessage replyPort: (mach_port_t*)replyPort maximumMIGReplySize: (size_t)maximumMIGReplySize flags:(uint64_t)flags demuxer:(xpc_pipe_mig_demux_f)demuxer;
+ (int)sendReply: (XPC_CLASS(dictionary)*)message;
+ (int)demux: (mach_msg_header_t*)request reply: (mach_msg_header_t*)reply demuxer: (xpc_pipe_mig_demux_f)demuxer;

- (instancetype)initForService: (const char*)serviceName withFlags: (uint64_t)flags;
- (instancetype)initWithPort: (mach_port_t)port flags: (uint64_t)flags;

- (int)sendMessage: (XPC_CLASS(dictionary)*)message withSynchronousReply: (xpc_object_t*)reply flags: (uint64_t)flags;
- (int)sendMessage: (XPC_CLASS(dictionary)*)message withReplyPort: (mach_port_t)replyPort;
- (int)forwardMessage: (XPC_CLASS(dictionary)*)message;
- (int)sendMessage: (XPC_CLASS(dictionary)*)message;

- (void)invalidate;

@end

#endif // _XPC_OBJECTS_PIPE_H_
