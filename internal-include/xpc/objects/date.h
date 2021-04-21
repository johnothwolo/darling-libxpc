#ifndef _XPC_OBJECTS_DATE_H_
#define _XPC_OBJECTS_DATE_H_

#import <xpc/objects/base.h>

XPC_CLASS_DECL(date);

struct xpc_date_s {
	struct xpc_object_s base;
	int64_t value;
	double absolute_value;
	bool is_absolute;
};

@interface XPC_CLASS_INTERFACE(date)

@property(assign) int64_t value;
@property(assign) double absoluteValue;

- (instancetype)initWithValue: (int64_t)value;
- (instancetype)initWithAbsoluteValue: (double)value;

@end

#endif // _XPC_OBJECTS_DATE_H_
