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

+ (instancetype)stringWithUTF8String: (const char*)string;
// non-NSString method
+ (instancetype)stringWithUTF8String: (const char*)string byteLength: (NSUInteger)byteLength;
// non-NSString method
+ (instancetype)stringWithUTF8StringNoCopy: (const char*)string freeWhenDone: (BOOL)freeIt;
+ (instancetype) XPC_PRINTF(1, 2) stringWithFormat: (const char*)format, ...;

// non-NSString method
- (instancetype)initWithUTF8String: (const char*)string byteLength: (NSUInteger)byteLength;
- (instancetype)initWithUTF8String: (const char*)string;
// non-NSString method
- (instancetype)initWithUTF8StringNoCopy: (const char*)string byteLength: (NSUInteger)byteLength freeWhenDone: (BOOL)freeIt;
// non-NSString method
- (instancetype)initWithUTF8StringNoCopy: (const char*)string freeWhenDone: (BOOL)freeIt;
- (instancetype) XPC_PRINTF(1, 2) initWithFormat: (const char*)format, ...;
- (instancetype) XPC_PRINTF(1, 0) initWithFormat: (const char*)format arguments: (va_list)args;

// non-NSString method
- (void)replaceStringWithString: (const char*)string;

// non-NSString method
- (instancetype)forceCopy;

// non-NSString method
- (void)appendString: (const char*)string;
// non-NSString method
- (void)appendString: (const char*)string length: (NSUInteger)length;
- (XPC_CLASS(string)*)stringByAppendingString: (const char*)string;
- (BOOL)isEqualToString: (const char*)string;

@end

@interface XPC_CLASS(string) (XPCStringPathExtensions)

@property(readonly) const char* pathExtension;
// NOTE: this property deviates from the NSString behavior in that it does not ignore trailing slashes
@property(readonly) const char* lastPathComponent;
@property(readonly, copy) XPC_CLASS(string)* stringByDeletingPathExtension;

- (XPC_CLASS(string)*)stringByResolvingSymlinksInPath;
- (XPC_CLASS(string)*)stringByAppendingPathComponent: (const char*)component;
// NOTE: this method deviates from the NSString behavior in that it does not ignore trailing slashes
- (XPC_CLASS(string)*)stringByDeletingLastPathComponent;


@end

#endif // _XPC_OBJECTS_STRING_H_
