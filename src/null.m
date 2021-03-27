#import <xpc/objects/null.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

XPC_CLASS_SYMBOL_DECL(null);

struct xpc_null_s _xpc_null = {
	.base = {
		XPC_GLOBAL_OBJECT_HEADER(null),
	},
};

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(null)

XPC_CLASS_HEADER(null);

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s>", xpc_class_name(self));
	return output;
}

+ (instancetype)null
{
	return XPC_CAST(null, &_xpc_null);
}

- (NSUInteger)hash
{
	return 0x804201026298ULL;
}

@end

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_null_create(void) {
	return [XPC_CLASS(null) null];
};
