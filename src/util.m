#import <xpc/util.h>
#import <objc/runtime.h>
#import <xpc/serialization.h>
#include <sys/reason.h>
#include <sys/syslog.h>
#import <xpc/objects/connection.h>

#ifndef XPC_LOG_TO_STDOUT_TOO
	#define XPC_LOG_TO_STDOUT_TOO 0
#endif

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

bool xpc_mach_port_is_dead(mach_port_t port) {
	mach_port_urefs_t refs = 0;
	if (MACH_PORT_VALID(port)) {
		mach_port_get_refs(mach_task_self(), port, MACH_PORT_RIGHT_DEAD_NAME, &refs);
	}
	return refs > 0;
};

kern_return_t xpc_mach_port_retain_right(mach_port_name_t port, mach_port_right_t right) {
	if (!MACH_PORT_VALID(port)) {
		return KERN_SUCCESS;
	}
	kern_return_t status = mach_port_mod_refs(mach_task_self(), port, right, 1);
	if (status == KERN_INVALID_RIGHT) {
		status = mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_DEAD_NAME, 1);
	}
	return status;
};

kern_return_t xpc_mach_port_release_right(mach_port_name_t port, mach_port_right_t right) {
	if (!MACH_PORT_VALID(port)) {
		return KERN_SUCCESS;
	}
	kern_return_t status = mach_port_mod_refs(mach_task_self(), port, right, -1);
	if (status == KERN_INVALID_RIGHT) {
		status = mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_DEAD_NAME, -1);
	}
	return status;
};

kern_return_t xpc_mach_port_retain_send(mach_port_t port) {
	return xpc_mach_port_retain_right(port, MACH_PORT_RIGHT_SEND);
};

kern_return_t xpc_mach_port_release_send(mach_port_t port) {
	return xpc_mach_port_release_right(port, MACH_PORT_RIGHT_SEND);
};

kern_return_t xpc_mach_port_make_send(mach_port_t port) {
	if (!MACH_PORT_VALID(port)) {
		return KERN_SUCCESS;
	}
	return mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
};

kern_return_t xpc_mach_port_retain_send_once(mach_port_t port) {
	return xpc_mach_port_retain_right(port, MACH_PORT_RIGHT_SEND_ONCE);
};

kern_return_t xpc_mach_port_release_send_once(mach_port_t port) {
	return xpc_mach_port_release_right(port, MACH_PORT_RIGHT_SEND_ONCE);
};

kern_return_t xpc_mach_port_retain_send_any(mach_port_t port) {
	kern_return_t status1 = xpc_mach_port_retain_send(port);
	kern_return_t status2 = xpc_mach_port_retain_send_once(port);
	// if either one of them succeeds, consider it a success
	return (status1 == KERN_SUCCESS || status2 == KERN_SUCCESS) ? KERN_SUCCESS : status1;
};

kern_return_t xpc_mach_port_release_send_any(mach_port_t port) {
	kern_return_t status1 = xpc_mach_port_release_send(port);
	kern_return_t status2 = xpc_mach_port_release_send_once(port);
	// if either one of them succeeds, consider it a success
	return (status1 == KERN_SUCCESS || status2 == KERN_SUCCESS) ? KERN_SUCCESS : status1;
};

kern_return_t xpc_mach_port_release_receive(mach_port_t port) {
	return xpc_mach_port_release_right(port, MACH_PORT_RIGHT_RECEIVE);
};

mach_port_right_t xpc_mach_msg_type_name_to_port_right(mach_msg_type_name_t type_name) {
	switch (type_name) {
		case MACH_MSG_TYPE_MOVE_RECEIVE:
			return MACH_PORT_RIGHT_RECEIVE;
		case MACH_MSG_TYPE_MOVE_SEND:
		case MACH_MSG_TYPE_COPY_SEND:
		case MACH_MSG_TYPE_MAKE_SEND:
			return MACH_PORT_RIGHT_SEND;
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:
			return MACH_PORT_RIGHT_SEND_ONCE;
	}
	return (mach_port_right_t)-1;
};

mach_port_t xpc_mach_port_create_receive(void) {
	mach_port_t port = MACH_PORT_NULL;
	if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) != KERN_SUCCESS) {
		return MACH_PORT_NULL;
	}
	return port;
};

mach_port_t xpc_mach_port_create_send_receive(void) {
	mach_port_t port = xpc_mach_port_create_receive();
	if (!MACH_PORT_VALID(port)) {
		return MACH_PORT_NULL;
	}
	if (xpc_mach_port_make_send(port) != KERN_SUCCESS) {
		xpc_mach_port_release_receive(port);
		return MACH_PORT_NULL;
	}
	return port;
};

bool xpc_mach_port_is_send_once(mach_port_t port) {
	mach_port_urefs_t refs = 0;
	if (MACH_PORT_VALID(port)) {
		mach_port_get_refs(mach_task_self(), port, MACH_PORT_RIGHT_SEND_ONCE, &refs);
	}
	return refs > 0;
};

XPC_NORETURN
void _xpc_abortv(const char* function, const char* file, size_t line, const char* reason_format, va_list args) {
	char* message = NULL;
	char* reason = NULL;
	if (reason_format == NULL) {
		reason = strdup("aborting for unknown reason");
	} else {
		vasprintf(&reason, reason_format, args);
	}
	asprintf(&message, "libxpc: %s:%zu: %s: %s", file, line, function, reason);
	free(reason);
	abort_with_reason(OS_REASON_LIBXPC, 0, message, 0);
	free(message); // unreachable
};

XPC_NORETURN
void _xpc_abort(const char* function, const char* file, size_t line, const char* reason_format, ...) {
	va_list args;
	va_start(args, reason_format);
	_xpc_abortv(function, file, line, reason_format, args);
	va_end(args); // unreachable
};

XPC_NORETURN
void _xpc_assertion_failed(const char* function, const char* file, size_t line, const char* expression) {
	_xpc_abort(function, file, line, "assertion failed: %s", expression);
};

void _xpc_logv(const char* function, const char* file, size_t line, xpc_log_priority_t priority, const char* format, va_list args) {
	char* message = NULL;
	char* reason = NULL;
	vasprintf(&reason, format, args);
#if XPC_LOG_TO_STDOUT_TOO
	// also log to stdout
	printf("libxpc: %s:%zu: %s: %s\n", file, line, function, reason);
	fflush(stdout);
#endif
	syslog(priority, "libxpc: %s:%zu: %s: %s", file, line, function, reason);
	free(reason);
};

void _xpc_log(const char* function, const char* file, size_t line, xpc_log_priority_t priority, const char* format, ...) {
	va_list args;
	va_start(args, format);
	_xpc_logv(function, file, line, priority, format, args);
	va_end(args);
};
