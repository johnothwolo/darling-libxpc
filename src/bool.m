#import <xpc/objects/bool.h>
#import <xpc/util.h>
#import <xpc/xpc.h>

#undef bool

// workaround because `xpc/xpc.h` uses `_xpc_bool_s` instead of `xpc_bool_s`
struct _xpc_bool_s {
	struct xpc_bool_s the_real_thing;
};

XPC_CLASS_SYMBOL_DECL(bool);

XPC_EXPORT
const struct _xpc_bool_s _xpc_bool_true = {
	.the_real_thing = {
		.base = {
			XPC_GLOBAL_OBJECT_HEADER(bool),
		},
		.value = true,
	},
};

XPC_EXPORT
const struct _xpc_bool_s _xpc_bool_false = {
	.the_real_thing = {
		.base = {
			XPC_GLOBAL_OBJECT_HEADER(bool),
		},
		.value = false,
	},
};

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(bool)

XPC_CLASS_HEADER(bool);

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s: %s>", xpc_class_name(self), self.value ? "TRUE" : "FALSE");
	return output;
}

- (BOOL)value
{
	XPC_THIS_DECL(bool);
	return this->value;
}

- (void)setValue: (BOOL)value
{
	XPC_THIS_DECL(bool);
	this->value = value;
}

- (instancetype)initWithValue: (BOOL)value
{
	if (self = [super init]) {
		XPC_THIS_DECL(bool);
		this->value = value;
	}
	return self;
}


+ (instancetype)boolForValue: (BOOL)value
{
	if (value) {
		return XPC_CAST(bool, &_xpc_bool_true);
	} else {
		return XPC_CAST(bool, &_xpc_bool_false);
	}
}

- (NSUInteger)hash
{
	XPC_THIS_DECL(bool);

	if (this->value) {
		return 0xb001;
	} else {
		return 0x100b;
	}
}

@end

#define bool _Bool

//
// C API
//

XPC_EXPORT
xpc_object_t xpc_bool_create(bool value) {
	return [XPC_CLASS(bool) boolForValue: value];
};

XPC_EXPORT
bool xpc_bool_get_value(xpc_object_t xbool) {
#undef bool
	TO_OBJC_CHECKED(bool, xbool, boolObj) {
#define bool _Bool
		return boolObj.value;
	}
	return false;
};

//
// private C API
//

XPC_EXPORT
xpc_object_t _xpc_bool_create_distinct(bool value) {
	return [[XPC_CLASS(bool) alloc] initWithValue: value];
};

XPC_EXPORT
void _xpc_bool_set_value(xpc_object_t xbool, bool value) {
#undef bool
	TO_OBJC_CHECKED(bool, xbool, boolObj) {
#define bool _Bool
		boolObj.value = value;
	}
};
