#import <xpc/objects/string.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/serialization.h>

XPC_CLASS_SYMBOL_DECL(string);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(string)

XPC_CLASS_HEADER(string);

+ (instancetype)stringWithUTF8String: (const char*)string
{
	return [[[[self class] alloc] initWithUTF8String: string] autorelease];
}

+ (instancetype)stringWithUTF8String: (const char*)string byteLength: (NSUInteger)byteLength
{
	return [[[[self class] alloc] initWithUTF8String: string byteLength: byteLength] autorelease];
}

+ (instancetype)stringWithUTF8StringNoCopy: (const char*)string freeWhenDone: (BOOL)freeIt
{
	return [[[[self class] alloc] initWithUTF8StringNoCopy: string freeWhenDone: freeIt] autorelease];
}

+ (instancetype)stringWithFormat: (const char*)format, ...
{
	va_list args;
	va_start(args, format);
	XPC_CLASS(string)* result = [[[[self class] alloc] initWithFormat: format arguments: args] autorelease];
	va_end(args);
	return result;
}

- (void)dealloc
{
	XPC_THIS_DECL(string);
	if (this->freeWhenDone && this->string) {
		free((void*)this->string);
	}
	[super dealloc];
}

- (char*)xpcDescription
{
	char* output = NULL;
	if (self.UTF8String) {
		asprintf(&output, "<%s: %s>", xpc_class_name(self), self.UTF8String);
	} else {
		asprintf(&output, "<%s: NULL>", xpc_class_name(self));
	}
	return output;
}

- (NSUInteger)byteLength
{
	XPC_THIS_DECL(string);
	if (this->byteLength == NSUIntegerMax) {
		return strlen(this->string);
	}
	return this->byteLength;
}

- (const char*)UTF8String
{
	XPC_THIS_DECL(string);
	return this->string;
}

- (instancetype)init
{
	return [self initWithUTF8String: "" byteLength: 0];
}

- (instancetype)initWithUTF8String: (const char*)string byteLength: (NSUInteger)byteLength
{
	if (self = [super init]) {
		XPC_THIS_DECL(string);

		this->byteLength = byteLength;
		char* buf = malloc(this->byteLength + 1);
		if (!buf) {
			[self release];
			return nil;
		}
		strncpy(buf, string, this->byteLength);
		buf[this->byteLength] = '\0';
		this->string = buf;
		this->freeWhenDone = YES;
	}
	return self;
}

- (instancetype)initWithUTF8String: (const char*)string
{
	return [self initWithUTF8String: string byteLength: strlen(string)];
}

- (instancetype)initWithUTF8StringNoCopy: (const char*)string byteLength: (NSUInteger)byteLength freeWhenDone: (BOOL)freeIt
{
	if (self = [super init]) {
		XPC_THIS_DECL(string);

		this->byteLength = byteLength;
		this->string = string;
		this->freeWhenDone = freeIt;
	}
	return self;
}

- (instancetype)initWithUTF8StringNoCopy: (const char*)string freeWhenDone: (BOOL)freeIt
{
	return [self initWithUTF8StringNoCopy: string byteLength: strlen(string) freeWhenDone: freeIt];
}

- (instancetype)initWithFormat: (const char*)format, ...
{
	va_list args;
	va_start(args, format);
	self = [self initWithFormat: format arguments: args];
	va_end(args);
	return self;
}

- (instancetype)initWithFormat: (const char*)format arguments: (va_list)args
{
	if (self = [super init]) {
		XPC_THIS_DECL(string);

		vasprintf((char**)&this->string, format, args);
		if (!this->string) {
			[self release];
			return nil;
		}
		this->byteLength = strlen(this->string);
		this->freeWhenDone = YES;
	}
	return self;
}

- (void)replaceStringWithString: (const char*)string
{
	XPC_THIS_DECL(string);
	size_t newByteLength = strlen(string);
	char* newString = malloc(newByteLength + 1);
	if (!newString) {
		return;
	}
	if (this->freeWhenDone && this->string) {
		free((void*)this->string);
	}
	this->byteLength = newByteLength;
	strncpy(newString, string, this->byteLength);
	newString[this->byteLength] = '\0';
	this->string = newString;
	this->freeWhenDone = true;
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(string);
	return xpc_raw_data_hash(this->string, self.byteLength + 1);
}

- (instancetype)forceCopy
{
	return [[[self class] alloc] initWithUTF8String: self.UTF8String];
}

- (void)appendString: (const char*)string length: (NSUInteger)extraByteLength
{
	if (extraByteLength == 0) {
		return;
	}
	XPC_THIS_DECL(string);
	size_t oldByteLength = self.byteLength;
	size_t newByteLength = oldByteLength + extraByteLength;
	char* newString = malloc(newByteLength + 1);
	if (!newString) {
		return;
	}
	strncpy(newString, this->string, oldByteLength);
	if (this->freeWhenDone && this->string) {
		free((void*)this->string);
	}
	this->byteLength = newByteLength;
	strncpy(newString + oldByteLength, string, extraByteLength);
	newString[newByteLength] = '\0';
	this->string = newString;
	this->freeWhenDone = true;
}

- (void)appendString: (const char*)string
{
	return [self appendString: string length: strlen(string)];
}

- (XPC_CLASS(string)*)stringByAppendingString: (const char*)string
{
	return [[self class] stringWithFormat: "%s/%s", self.UTF8String, string];
}

- (BOOL)isEqualToString: (const char*)string
{
	return strcmp(self.UTF8String, string) == 0;
}

@end

@implementation XPC_CLASS(string) (XPCSerialization)

- (BOOL)serializable
{
	return YES;
}

- (NSUInteger)serializationLength
{
	return xpc_serial_padded_length(sizeof(xpc_serial_type_t)) + xpc_serial_padded_length(sizeof(uint32_t)) + xpc_serial_padded_length(self.byteLength + 1);
}

+ (instancetype)deserialize: (XPC_CLASS(deserializer)*)deserializer
{
	XPC_CLASS(string)* result = nil;
	xpc_serial_type_t type = XPC_SERIAL_TYPE_INVALID;
	uint32_t length = 0;
	const char* string = NULL;

	if (![deserializer readU32: &type]) {
		goto error_out;
	}
	if (type != XPC_SERIAL_TYPE_STRING) {
		goto error_out;
	}

	if (![deserializer readU32: &length]) {
		goto error_out;
	}

	if (![deserializer readString: &string]) {
		goto error_out;
	}

	// maybe we should check if the string length matches the reported length

	result = [[[self class] alloc] initWithUTF8String: string];

	return result;

error_out:
	if (result != nil) {
		[result release];
	}
	return nil;
}

- (BOOL)serialize: (XPC_CLASS(serializer)*)serializer
{
	if (![serializer writeU32: XPC_SERIAL_TYPE_STRING]) {
		goto error_out;
	}

	if (![serializer writeU32: self.byteLength]) {
		goto error_out;
	}

	if (![serializer writeString: self.UTF8String]) {
		goto error_out;
	}

	return YES;

error_out:
	return NO;
}

@end

@implementation XPC_CLASS(string) (XPCStringPathExtensions)

- (const char*)pathExtension
{
	char* lastDot = strrchr(self.UTF8String, '.');
	return lastDot ? lastDot + 1 : "";
}

- (const char*)lastPathComponent
{
	char* lastSlash = strrchr(self.UTF8String, '/');
	return lastSlash ? lastSlash + 1 : "";
}

- (XPC_CLASS(string)*)stringByDeletingPathExtension
{
	char* lastDot = strrchr(self.UTF8String, '.');
	return (lastDot) ? [[self class] stringWithUTF8String: self.UTF8String byteLength: lastDot - self.UTF8String] : [self forceCopy];
}

- (XPC_CLASS(string)*)stringByResolvingSymlinksInPath
{
	char* resolved = realpath(self.UTF8String, NULL);
	return resolved ? [[self class] stringWithUTF8StringNoCopy: resolved freeWhenDone: YES] : nil;
}

- (XPC_CLASS(string)*)stringByAppendingPathComponent: (const char*)component
{
	if (self.byteLength == 0) {
		return [[self class] stringWithUTF8String: component];
	} else if (self.UTF8String[self.byteLength - 1] == '/') {
		return [self stringByAppendingString: component];
	} else {
		return [[self class] stringWithFormat: "%s/%s", self.UTF8String, component];
	}
}

- (XPC_CLASS(string)*)stringByDeletingLastPathComponent
{
	char* lastSlash = strrchr(self.UTF8String, '/');
	if (lastSlash && lastSlash > self.UTF8String) {
		*lastSlash = '\0'; // temporarily shorten the string for the copy...
		XPC_CLASS(string)* result = [[self class] stringWithUTF8String: self.UTF8String];
		*lastSlash = '/'; // ...and then restore it
		return result;
	} else if (lastSlash == self.UTF8String) {
		// if it's the one for the root, return the root
		return [[self class] stringWithUTF8String: "/"];
	} else {
		// otherwise, if it's not present, return an empty string
		return [[self class] stringWithUTF8String: ""];
	}
}

@end

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_string_create(const char* string) {
	return [[XPC_CLASS(string) alloc] initWithUTF8String: string];
};

XPC_EXPORT
xpc_object_t xpc_string_create_with_format(const char* format, ...) {
	va_list args;
	va_start(args, format);
	xpc_object_t string = xpc_string_create_with_format_and_arguments(format, args);
	va_end(args);
	return string;
};

XPC_EXPORT
xpc_object_t xpc_string_create_with_format_and_arguments(const char* format, va_list args) {
	return [[XPC_CLASS(string) alloc] initWithFormat: format arguments: args];
};

XPC_EXPORT
size_t xpc_string_get_length(xpc_object_t xstring) {
	TO_OBJC_CHECKED(string, xstring, string) {
		return string.byteLength;
	}
	return 0;
};

XPC_EXPORT
const char* xpc_string_get_string_ptr(xpc_object_t xstring) {
	TO_OBJC_CHECKED(string, xstring, string) {
		return string.UTF8String;
	}
	return NULL;
};

//
// private C API
//

XPC_EXPORT
void _xpc_string_set_value(xpc_object_t xstring, const char* new_string) {
	TO_OBJC_CHECKED(string, xstring, string) {
		[string replaceStringWithString: new_string];
	}
};

XPC_EXPORT
xpc_object_t xpc_string_create_no_copy(const char* string) {
	return [[XPC_CLASS(string) alloc] initWithUTF8StringNoCopy: string freeWhenDone: NO];
};
