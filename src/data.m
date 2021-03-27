#import <xpc/objects/data.h>
#import <xpc/util.h>
#define __DISPATCH_INDIRECT__
#import <dispatch/data_private.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(data);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(data)

XPC_CLASS_HEADER(data);

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s: %p, %zu byte%s>", xpc_class_name(self), self, self.length, self.length == 1 ? "" : "s");
	return output;
}

- (void)dealloc
{
	XPC_THIS_DECL(data);
	dispatch_release(this->data);
	[super dealloc];
}

- (const void*)bytes
{
	XPC_THIS_DECL(data);
	return dispatch_data_get_flattened_bytes_4libxpc(this->data);
}

- (NSUInteger)length
{
	XPC_THIS_DECL(data);
	return dispatch_data_get_size(this->data);
}

- (instancetype)initWithBytes: (const void*)bytes length: (NSUInteger)length
{
	if (self = [super init]) {
		XPC_THIS_DECL(data);
		this->data = dispatch_data_create(bytes, length, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
	}
	return self;
}

- (instancetype)initWithDispatchData: (dispatch_data_t)data
{
	if (self = [super init]) {
		XPC_THIS_DECL(data);
		this->data = data;
		dispatch_retain(this->data);
	}
	return self;
}

- (NSUInteger)getBytes: (void*)buffer length: (NSUInteger)length
{
	XPC_THIS_DECL(data);
	length = MIN(length, self.length);
	memcpy(buffer, self.bytes, length);
	return length;
}

- (void)replaceBytesWithBytes: (const void*)bytes length: (NSUInteger)length
{
	XPC_THIS_DECL(data);
	dispatch_release(this->data);
	this->data = dispatch_data_create(bytes, length, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(data);
	__block NSUInteger result = 0;
	dispatch_data_apply(this->data, ^bool (dispatch_data_t region, size_t offset, const void* buffer, size_t size) {
		result += xpc_raw_data_hash(buffer, size);
		return true;
	});
	return result;
}

@end

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_data_create(const void* bytes, size_t length) {
	return [[XPC_CLASS(data) alloc] initWithBytes: bytes length: length];
};

XPC_EXPORT
xpc_object_t xpc_data_create_with_dispatch_data(dispatch_data_t ddata) {
	return [[XPC_CLASS(data) alloc] initWithDispatchData: ddata];
};

XPC_EXPORT
size_t xpc_data_get_length(xpc_object_t xdata) {
	TO_OBJC_CHECKED(data, xdata, data) {
		return data.length;
	}
	return 0;
};

XPC_EXPORT
const void* xpc_data_get_bytes_ptr(xpc_object_t xdata) {
	TO_OBJC_CHECKED(data, xdata, data) {
		return data.bytes;
	}
	return NULL;
};

XPC_EXPORT
size_t xpc_data_get_bytes(xpc_object_t xdata, void* buffer, size_t offset, size_t length) {
	TO_OBJC_CHECKED(data, xdata, data) {
		return [data getBytes: ((char*)buffer + offset) length: length];
	}
	return 0;
};

//
// private C API
//

XPC_EXPORT
void _xpc_data_set_value(xpc_object_t xdata, const void* bytes, size_t length) {
	TO_OBJC_CHECKED(data, xdata, data) {
		[data replaceBytesWithBytes: bytes length: length];
	}
};

XPC_EXPORT
size_t xpc_data_get_inline_max(xpc_object_t xdata) {
	// not a stub
	// just returns 0
	return 0;
};
