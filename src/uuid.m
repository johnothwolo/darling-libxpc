#import <xpc/objects/uuid.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

#define UUID_STRING_LENGTH 36

XPC_CLASS_SYMBOL_DECL(uuid);

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(uuid)

XPC_CLASS_HEADER(uuid);

- (char*)xpcDescription
{
	char* output = NULL;
	char asString[UUID_STRING_LENGTH + 1];
	uuid_unparse(self.bytes, asString);
	asprintf(&output, "<%s: %s>", xpc_class_name(self), asString);
	return output;
}

- (uint8_t*)bytes
{
	XPC_THIS_DECL(uuid);
	return this->value;
}

- (instancetype)initWithBytes: (const uint8_t*)bytes
{
	if (self = [super init]) {
		XPC_THIS_DECL(uuid);
		memcpy(this->value, bytes, sizeof(this->value));
	}
	return self;
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(uuid);
	return xpc_raw_data_hash(this->value, sizeof(this->value));
}

@end

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_uuid_create(const uuid_t uuid) {
	return [[XPC_CLASS(uuid) alloc] initWithBytes: uuid];
};

XPC_EXPORT
const uint8_t* xpc_uuid_get_bytes(xpc_object_t xuuid) {
	TO_OBJC_CHECKED(uuid, xuuid, uuid) {
		return uuid.bytes;
	}
	return NULL;
};
