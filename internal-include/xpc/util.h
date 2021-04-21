#ifndef _XPC_UTIL_H_
#define _XPC_UTIL_H_

#import <xpc/internal_base.h>
#import <xpc/objects/base.h>

#include <stdlib.h>
#include <stdbool.h>
#include <mach/mach.h>
#include <sys/syslog.h>

/**
 * Expands to an expression that evaluates to `true` if the given expression is an object of the given XPC class, or `false` otherwise.
 */
#define XPC_CHECK(className, expression) ([(expression) isKindOfClass: [XPC_CLASS(className) class]])

/**
 * Defines `objcName` by casting `origName`, but also checks to ensure it's not `nil`
 * and to ensure it is an XPC object of the given `className`.
 *
 * You should call it with a (possibly braced) statement of what to do when it passes these checks.
 * You can optionally append an `else` clause for what to do when it fails these checks.
 *
 * @code
 * 	xpc_object_t some_input = get_some_input();
 * 	TO_OBJC_CHECKED(string, some_input, some_checked_input) {
 * 		printf("Yay! It's a string! Look: %s\n", some_checked_input.description.UTF8String); // note that this assumes NSString is loaded
 * 	} else {
 * 		printf("Oh no, you messed up and gave me an object that wasn't a string :(\n");
 * 	}
 * @endcode
 */
#define TO_OBJC_CHECKED(className, origName, objcName) \
	XPC_CLASS(className)* objcName = XPC_CAST(className, origName); \
	if (objcName && [objcName isKindOfClass: [XPC_CLASS(className) class]])

/**
 * Like `TO_OBJC_CHECKED`, but the condition checks for failure to pass the checks.
 *
 * @see TO_OBJC_CHECKED
 */
#define TO_OBJC_CHECKED_ON_FAIL(className, origName, objcName) \
	XPC_CLASS(className)* objcName = XPC_CAST(className, origName); \
	if (!objcName || ![objcName isKindOfClass: [XPC_CLASS(className) class]])

/**
 * Special `retain` variant for collection classes like dictionaries and arrays.
 *
 * This is necessary because collections do not retain certain types of objects
 * and just store them weakly.
 *
 * @returns The object passed in, possibly with an increased reference count.
 */
XPC_CLASS(object)* xpc_retain_for_collection(XPC_CLASS(object)* object);

/**
 * Special `release` variant for collection classes like dictionaries and arrays.
 *
 * @see xpc_retain_for_collection
 *
 * @returns The object passed in, possibly with a decreased reference count.
 */
void xpc_release_for_collection(XPC_CLASS(object)* object);

/**
 * Maps the given object to a class name.
 *
 * @returns The mapped class name for the object.
 */
const char* xpc_class_name(XPC_CLASS(object)* object);

/**
 * Creates a string that is an indented copy of the given string.
 *
 * @returns A string that must be freed.
 */
char* xpc_description_indent(const char* description, bool indentFirstLine);

/**
 * Produces a hash from the given data.
 *
 * @returns A hash of the input data.
 */
size_t xpc_raw_data_hash(const void* data, size_t data_length);

/**
 * Checks if the given port is dead.
 */
bool xpc_mach_port_is_dead(mach_port_t port);

/**
 * Increments the reference count on the given right for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 *
 * @note Certain rights (like receive rights) cannot be retained, only released. This a Mach limitation.
 */
kern_return_t xpc_mach_port_retain_right(mach_port_name_t port, mach_port_right_t right);

/**
 * Decrements the reference count on the given right for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 */
kern_return_t xpc_mach_port_release_right(mach_port_name_t port, mach_port_right_t right);

/**
 * Increments the reference count on the send right for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 */
kern_return_t xpc_mach_port_retain_send(mach_port_t port);

/**
 * Decrements the reference count on the send right for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 */
kern_return_t xpc_mach_port_release_send(mach_port_t port);

/**
 * Creates or retains a send right in the given port.
 */
kern_return_t xpc_mach_port_make_send(mach_port_t port);

/**
 * Increments the reference count on the send-once right for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 */
kern_return_t xpc_mach_port_retain_send_once(mach_port_t port);

/**
 * Decrements the reference count on the send-once right for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 */
kern_return_t xpc_mach_port_release_send_once(mach_port_t port);

/**
 * Increments the reference count on all types of send rights (i.e. send and send-once) for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 */
kern_return_t xpc_mach_port_retain_send_any(mach_port_t port);

/**
 * Decrements the reference count on all types of send rights (i.e. send and send-once) for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 */
kern_return_t xpc_mach_port_release_send_any(mach_port_t port);

/**
 * Decrements the reference count on the receive right for the given port.
 *
 * Works regardless of whether the port is dead or not.
 *
 * Can safely be called on `MACH_PORT_NULL` and `MACH_PORT_DEAD`, which will just result in a no-op.
 *
 * @note There is no corresponding retain operation for receive ports because Mach limits receive ports to a single reference.
 */
kern_return_t xpc_mach_port_release_receive(mach_port_t port);

/**
 * Maps the given message type name to a port right type.
 */
mach_port_right_t xpc_mach_msg_type_name_to_port_right(mach_msg_type_name_t type_name);

/**
 * Creates a new receive port.
 */
mach_port_t xpc_mach_port_create_receive(void);

/**
 * Creates a new send-receive port.
 */
mach_port_t xpc_mach_port_create_send_receive(void);

/**
 * Checks if the given port contains a send-once right.
 */
bool xpc_mach_port_is_send_once(mach_port_t port);

/**
 * Aborts the current process, with an optional reason.
 */
XPC_NORETURN XPC_PRINTF(4, 5)
void _xpc_abort(const char* function, const char* file, size_t line, const char* reason_format, ...);

/**
 * @see _xpc_abort
 */
XPC_NORETURN XPC_PRINTF(4, 0)
void _xpc_abortv(const char* function, const char* file, size_t line, const char* reason_format, va_list args);

/**
 * Aborts the current process, with an optional reason. This macro automatically fills in most of the arguments to `_xpc_abort`.
 */
#define xpc_abort(...) _xpc_abort(__PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @see xpc_abort
 */
#define xpc_abortv(...) _xpc_abortv(__PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

/**
 * Aborts the current process due to a failed assertion.
 */
XPC_NORETURN
void _xpc_assertion_failed(const char* function, const char* file, size_t line, const char* expression);

#if XPC_DEBUG
	#define xpc_assert(x) do { \
			if (!(x)) _xpc_assertion_failed(__PRETTY_FUNCTION__, __FILE__, __LINE__, #x);\
		} while (0)
#else
	#define xpc_assert(x)
#endif

#define XPC_LOG_DEBUG   LOG_DEBUG
#define XPC_LOG_WARNING LOG_WARNING
#define XPC_LOG_ERROR   LOG_ERR
#define XPC_LOG_NOTICE  LOG_NOTICE
#define XPC_LOG_INFO    LOG_INFO

typedef int xpc_log_priority_t;

/**
 * Logs a message with the given priority.
 */
XPC_PRINTF(5, 6)
void _xpc_log(const char* function, const char* file, size_t line, xpc_log_priority_t priority, const char* format, ...);

/**
 * @see _xpc_log
 */
XPC_PRINTF(5, 0)
void _xpc_logv(const char* function, const char* file, size_t line, xpc_log_priority_t priority, const char* format, va_list args);

/**
 * Logs a message with the given priority. This macro automatically fills in most of the arguments to `_xpc_log`.
 */
#define xpc_log(...) _xpc_log(__PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @see xpc_log
 */
#define xpc_logv(...) _xpc_logv(__PRETTY_FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

/**
 * Prints a message indicating that a stub was called.
 */
void _xpc_stub(const char* function, const char* file, size_t line);

/**
 * Prints a message indicating that a stub was called. This macro automatically fills in the arguments to `_xpc_stub`.
 */
#define xpc_stub() _xpc_stub(__PRETTY_FUNCTION__, __FILE__, __LINE__)

/**
 * `true` if the given path exists and points to a regular file, `false` otherwise.
 */
bool xpc_path_is_file(const char* path);

/**
 * Returns a copy of the path for the main executable. Must be freed.
 */
char* xpc_copy_main_executable_path(void);

#endif // _XPC_UTIL_H_
