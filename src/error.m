#import <xpc/objects/error.h>
#import <xpc/objects/string.h>
#import <xpc/xpc.h>
#import <xpc/util.h>

XPC_CLASS_SYMBOL_DECL(error);

#define XPCErrorDescriptionKey "XPCErrorDescription"
const char* const _xpc_error_key_description = XPCErrorDescriptionKey;

// workaround like in `bool.m`
struct _xpc_dictionary_s {
	struct xpc_error_s the_real_thing;
};

#define XPC_ERROR_DEFINITION(_name, _description) \
	struct xpc_string_s _xpc_error_ ## _name ## _entry_string = { \
		.base = { \
			XPC_GLOBAL_OBJECT_HEADER(string), \
		}, \
		.byteLength = NSUIntegerMax, \
		.string = _description, \
		.freeWhenDone = false \
	}; \
	extern struct _xpc_dictionary_s _xpc_error_ ## _name; \
	struct xpc_dictionary_entry_s _xpc_error_ ## _name ## _entry = { \
		.link = { \
			.le_next = NULL, \
			.le_prev = &LIST_FIRST(&_xpc_error_ ## _name.the_real_thing.base.head), \
		}, \
		.object = XPC_CAST(string, &_xpc_error_ ## _name ## _entry_string), \
		.name = XPCErrorDescriptionKey, \
	}; \
	XPC_EXPORT struct _xpc_dictionary_s _xpc_error_ ## _name = { \
		.the_real_thing = { \
			.base = { \
				.base = { \
					XPC_GLOBAL_OBJECT_HEADER(error), \
				}, \
				.size = 1, \
				.head = { \
					.lh_first = &_xpc_error_ ## _name ## _entry, \
				}, \
			}, \
		}, \
	};

XPC_ERROR_DEFINITION(connection_interrupted, "Connection interrupted");
XPC_ERROR_DEFINITION(connection_invalid, "Connection invalid");
XPC_ERROR_DEFINITION(termination_imminent, "Termination imminent");

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(error)

XPC_CLASS_HEADER(error);

- (char*)xpcDescription
{
	char* output = NULL;
	asprintf(&output, "<%s: %s>", xpc_class_name(self), [self stringForKey: XPCErrorDescriptionKey].UTF8String);
	return output;
}

@end

//
// C API
//

XPC_EXPORT
const char* xpc_strerror(int error) {
	return NULL;
};
