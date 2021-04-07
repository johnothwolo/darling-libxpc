#import <xpc/objects/string.h>
#import <xpc/util.h>
#import <xpc/xpc.h>
#import <xpc/serialization.h>

XPC_CLASS_SYMBOL_DECL(string);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(string)

XPC_CLASS_HEADER(string);

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

- (instancetype)initWithUTF8String: (const char*)string
{
	if (self = [super init]) {
		XPC_THIS_DECL(string);

		this->byteLength = strlen(string);
		char* buf = malloc(this->byteLength + 1);
		if (!buf) {
			[self release];
			return nil;
		}
		strncpy(buf, string, this->byteLength + 1);
		this->string = buf;
		this->freeWhenDone = YES;
	}
	return self;
}

- (instancetype)initWithUTF8StringNoCopy: (const char*)string freeWhenDone: (BOOL)freeIt
{
	if (self = [super init]) {
		XPC_THIS_DECL(string);

		this->byteLength = strlen(string);
		this->string = string;
		this->freeWhenDone = freeIt;
	}
	return self;
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
	strncpy(newString, string, this->byteLength + 1);
	this->string = newString;
	this->freeWhenDone = true;
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(string);
	return xpc_raw_data_hash(this->string, self.byteLength + 1);
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
