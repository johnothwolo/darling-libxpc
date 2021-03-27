#import <xpc/util.h>
#import <objc/runtime.h>

XPC_CLASS(object)* xpc_retain_for_collection(XPC_CLASS(object)* object) {
	// connections aren't retained by collections
	if ([object isKindOfClass: [XPC_CLASS(connection) class]]) {
		return object;
	}
	return [object retain];
};

void xpc_release_for_collection(XPC_CLASS(object)* object) {
	if ([object isKindOfClass: [XPC_CLASS(connection) class]]) {
		return;
	}
	return [object release];
};

const char* xpc_class_name(XPC_CLASS(object)* object) {
	return class_getName([object class]);
};

char* xpc_description_indent(const char* description, bool indentFirstLine) {
	size_t newSize = 0;
	for (size_t i = 0; description[i] != '\0'; ++i) {
		++newSize;
		if (description[i] == '\n') {
			++newSize;
		}
	}
	if (indentFirstLine) {
		++newSize;
	}
	char* newDescription = malloc(sizeof(char) * (newSize + 1));
	if (indentFirstLine) {
		newDescription[0] = '\t';
	}
	for (size_t i = 0, j = indentFirstLine ? 1 : 0; description[i] != '\0'; ++i, ++j) {
		newDescription[j] = description[i];
		if (description[i] == '\n') {
			newDescription[++j] = '\t';
		}
	}
	newDescription[newSize] = '\0';
	return newDescription;
};

size_t xpc_raw_data_hash(const void* data, size_t data_length) {
	if (data_length == 0) {
		return 0x1505;
	}
	size_t result = 0;
	for (size_t i = 0; i < data_length; ++i) {
		uint8_t current = ((uint8_t*)data)[i];
		if (current == 0) {
			break;
		}
		result = (result * 0x21) + current;
	}
	return result;
};

kern_return_t xpc_mach_port_retain_right(mach_port_name_t port, mach_port_right_t right) {
	return mach_port_mod_refs(mach_task_self(), port, right, 1);
};

kern_return_t xpc_mach_port_release_right(mach_port_name_t port, mach_port_right_t right) {
	return mach_port_mod_refs(mach_task_self(), port, right, -1);
};
