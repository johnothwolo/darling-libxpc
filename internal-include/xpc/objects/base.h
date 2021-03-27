#ifndef _XPC_OBJECTS_BASE_H_
#define _XPC_OBJECTS_BASE_H_

#import <os/object_private.h>
#import <objc/NSObject.h>

#ifndef __XPC_INDIRECT__
OS_OBJECT_DECL(xpc_object);
#endif

#define __XPC_INDIRECT__
#import <xpc/base.h>

#define XPC_CLASS(name) OS_OBJECT_CLASS(xpc_ ## name)

#define XPC_CLASS_DECL(name) OS_OBJECT_DECL_SUBCLASS(xpc_ ## name, xpc_object)

#define XPC_CLASS_INTERFACE(name) XPC_CLASS(name) : XPC_CLASS(object) <XPC_CLASS(name)>

#define XPC_CAST(name, object) ((XPC_CLASS(name)*)object)

#define XPC_CLASS_HEADER(name) \
	OS_OBJECT_NONLAZY_CLASS_LOAD \
	+ (NSUInteger)_instanceSize \
	{ \
		return sizeof(struct xpc_ ## name ## _s); \
	}

#define XPC_THIS(name) ((struct xpc_ ## name ## _s*)self)
#define XPC_THIS_DECL(name) struct xpc_ ## name ## _s* this = XPC_THIS(name)

#define XPC_WRAPPER_CLASS_DECL(name, type) \
	XPC_CLASS_DECL(name); \
	struct xpc_ ## name ## _s { \
		struct xpc_object_s base; \
		type value; \
	}; \
	@interface XPC_CLASS_INTERFACE(name) \
		@property(assign) type value; \
		- (instancetype)initWithValue: (type)value; \
	@end

#define XPC_WRAPPER_CLASS_IMPL(name, type, format) \
	XPC_CLASS_SYMBOL_DECL(name); \
	OS_OBJECT_NONLAZY_CLASS \
	@implementation XPC_CLASS(name) \
	XPC_CLASS_HEADER(name); \
	- (char*)xpcDescription \
	{ \
		char* output = NULL; \
		asprintf(&output, "<%s: " format ">", xpc_class_name(self), self.value); \
		return output; \
	} \
	- (type)value \
	{ \
		XPC_THIS_DECL(name); \
		return this->value; \
	} \
	- (void)setValue: (type)value \
	{ \
		XPC_THIS_DECL(name); \
		this->value = value; \
	} \
	- (instancetype)initWithValue: (type)value \
	{ \
		if (self = [super init]) { \
			XPC_THIS_DECL(name); \
			this->value = value; \
		} \
		return self; \
	} \
	- (NSUInteger)hash \
	{ \
		XPC_THIS_DECL(name); \
		return xpc_raw_data_hash(&this->value, sizeof(this->value)); \
	} \
	@end

// hack to create symbol aliases programmatically, because the `alias` attribute isn't supported on Darwin platforms
#define _CREATE_ALIAS(original, alias) __asm__(".globl " original "; .globl " alias "; .equiv " alias ", " original)

#define XPC_CLASS_SYMBOL(name) _xpc_type_ ## name
#define XPC_CLASS_SYMBOL_DECL(name) \
	XPC_EXPORT struct objc_class XPC_CLASS_SYMBOL(name); \
	_CREATE_ALIAS(OS_OBJC_CLASS_RAW_SYMBOL_NAME(XPC_CLASS(name)), "_" OS_STRINGIFY(XPC_CLASS_SYMBOL(name)))

#define XPC_OBJC_CLASS(name) ((Class)&XPC_CLASS_SYMBOL(name))

#define XPC_GLOBAL_OBJECT_HEADER(className) \
	.os_obj_isa = (const struct xpc_object_vtable_s*)XPC_OBJC_CLASS(className), \
	.os_obj_ref_cnt = _OS_OBJECT_GLOBAL_REFCNT, \
	.os_obj_xref_cnt = _OS_OBJECT_GLOBAL_REFCNT

//
// base XPC class
//

struct xpc_object_vtable_s {
	_OS_OBJECT_CLASS_HEADER();
};

struct xpc_object_s {
	_OS_OBJECT_HEADER(
		const struct xpc_object_vtable_s* os_obj_isa,
		os_obj_ref_cnt,
		os_obj_xref_cnt
	);
};

XPC_EXPORT
@interface XPC_CLASS(object) : OS_OBJECT_CLASS(object) <XPC_CLASS(object)>

+ (NSUInteger)_instanceSize;

// note that this method returns a string that must be freed
- (char*)xpcDescription;

@end

#endif // _XPC_OBJECTS_BASE_H_
