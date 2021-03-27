#ifndef _XPC_OBJECTS_DATA_H_
#define _XPC_OBJECTS_DATA_H_

#import <xpc/objects/base.h>
#import <dispatch/dispatch.h>

XPC_CLASS_DECL(data);

struct xpc_data_s {
	struct xpc_object_s base;
	dispatch_data_t data;
};

@interface XPC_CLASS_INTERFACE(data)

// this API is modeled after NSMutableData

// NOTE: deviates from NSData by always returning a pointer to the internal buffer, even if `length` is 0
@property(readonly) const void* bytes;
@property(readonly) NSUInteger length;

- (instancetype)initWithBytes: (const void*)bytes length: (NSUInteger)length;
- (instancetype)initWithDispatchData: (dispatch_data_t)data;

// NOTE: deviates from NSData by returning the number of bytes copied
- (NSUInteger)getBytes: (void*)buffer length: (NSUInteger)length;

// non-NSData method
- (void)replaceBytesWithBytes: (const void*)bytes length: (NSUInteger)length;

@end

#endif // _XPC_OBJECTS_DATA_H_
