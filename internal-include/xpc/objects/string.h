#ifndef _XPC_OBJECTS_STRING_H_
#define _XPC_OBJECTS_STRING_H_

#import <xpc/objects/base.h>
#include <stdarg.h>

XPC_CLASS_DECL(string);

struct xpc_string_s {
	struct xpc_object_s base;
	NSUInteger byteLength; // acts as an optional cache
	BOOL freeWhenDone;
	const char* string;
};

@interface XPC_CLASS_INTERFACE(string)

// this API is modeled after NSString

// non-NSString property; length of the string in bytes, not including the null terminator
@property(readonly) NSUInteger byteLength;
@property(readonly) const char* UTF8String;

- (instancetype)initWithUTF8String: (const char*)string;
// non-NSString method
- (instancetype)initWithUTF8StringNoCopy: (const char*)string freeWhenDone: (BOOL)freeIt;
- (instancetype)initWithFormat: (const char*)format, ...;
- (instancetype)initWithFormat: (const char*)format arguments: (va_list)args;

// non-NSString method
- (void)replaceStringWithString: (const char*)string;

@end

#endif // _XPC_OBJECTS_STRING_H_
