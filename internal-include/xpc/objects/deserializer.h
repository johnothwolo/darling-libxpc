#ifndef _XPC_OBJECTS_DESERIALIZER_H_
#define _XPC_OBJECTS_DESERIALIZER_H_

#import <xpc/objects/base.h>

#include <dispatch/dispatch.h>
#ifndef __DISPATCH_INDIRECT__
#define __DISPATCH_INDIRECT__ 1
#endif
#include <dispatch/mach_private.h>

#define MACH_MSG_RECV_DISPOSITION_FIRST MACH_MSG_TYPE_PORT_NAME
#define MACH_MSG_RECV_DISPOSITION_LAST  MACH_MSG_TYPE_PORT_SEND_ONCE
#define MACH_MSG_RECV_DISPOSITION_COUNT (MACH_MSG_RECV_DISPOSITION_LAST - MACH_MSG_RECV_DISPOSITION_FIRST + 1)

// NOTE: this class is not present in the official libxpc

XPC_CLASS_DECL(deserializer);

typedef struct xpc_deserial_port_array {
	mach_port_t* array;
	size_t length;
	size_t offset;
} xpc_deserial_port_array_t;

struct xpc_deserializer_s {
	struct xpc_object_s base;
	dispatch_mach_msg_t mach_msg;
	size_t length;
	size_t offset;
	const void* buffer;
	xpc_deserial_port_array_t port_arrays[MACH_MSG_RECV_DISPOSITION_COUNT];
};

@class XPC_CLASS(dictionary);

@interface XPC_CLASS_INTERFACE(deserializer)

@property(readonly) NSUInteger offset;

/**
 * The send(-once) port that can be used to reply to the remote peer that sent the message being deserialized.
 */
@property(readonly) mach_port_t remotePort;

/**
 * Creates a new, autoreleased deserializer for the given message.
 */
+ (instancetype)deserializerWithMessage: (dispatch_mach_msg_t)message;

/**
 * Transforms the given Mach message directly into an XPC dictionary.
 * Returns `nil` if the given message does not contain a dictionary as the root object or it is not a valid XPC message.
 *
 * This method will automatically populate remote peer information in the returned dictionary.
 *
 * The returned dictionary is autoreleased.
 *
 * @note This method consumes the message passed in (regardless of whether it succeeds or not),
 *       so you should not use it anymore after passing it to this method.
 */
+ (XPC_CLASS(dictionary)*)process: (dispatch_mach_msg_t)message;

/**
 * Initializes this deserializer using the given Mach message.
 * Returns `nil` if the given message is not a valid XPC message.
 *
 * @note This initializer consumes the message passed in (regardless of whether it succeeds or not),
 *       so you should not use it anymore after passing it to this initializer.
 */
- (instancetype)initWithMessage: (dispatch_mach_msg_t)message;

/**
 * Initializes this deserializer using the given Mach message, without expecting an XPC header (i.e. XPC magic and version).
 *
 * @note This initializer consumes the message passed in (regardless of whether it succeeds or not),
 *       so you should not use it anymore after passing it to this initializer.
 */
- (instancetype)initWithoutHeaderWithMessage: (dispatch_mach_msg_t)message;

// NOTE: all reads from the internal buffer are subject to padding,
//       so the number of bytes passed in might not be the same as number of bytes actually read.

- (BOOL)read: (void*)data length: (NSUInteger)length;
- (BOOL)consume: (NSUInteger)length region: (const void**)region;
- (BOOL)readString: (const char**)string;
- (BOOL)readU32: (uint32_t*)value;
- (BOOL)readU64: (uint64_t*)value;
- (BOOL)readPort: (mach_port_t*)port type: (mach_msg_type_name_t)type;
- (BOOL)readObject: (XPC_CLASS(object)**)object;

- (BOOL)peek: (void*)data length: (NSUInteger)length;
- (BOOL)peekNoCopy: (NSUInteger)length region: (const void**)region;
- (BOOL)peekString: (const char**)string;
- (BOOL)peekU32: (uint32_t*)value;
- (BOOL)peekU64: (uint64_t*)value;

// ports are non-trivial to read, so they don't have a peeking method.
// objects might need to read ports, so they also don't have a peeking method.

@end

#endif // _XPC_OBJECTS_DESERIALIZER_H_
