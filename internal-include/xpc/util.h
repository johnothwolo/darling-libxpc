#ifndef _XPC_UTIL_H_
#define _XPC_UTIL_H_

#import <xpc/objects/base.h>
#import <xpc/objects/connection.h>

#include <stdlib.h>
#include <stdbool.h>
#include <mach/mach.h>

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
 * @see `TO_OBJC_CHECKED`
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
 * @see `xpc_retain_for_collection`
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
 * Increments the reference count on the given right for the given port.
 */
kern_return_t xpc_mach_port_retain_right(mach_port_name_t port, mach_port_right_t right);

/**
 * Decrements the reference count on the given right for the given port.
 */
kern_return_t xpc_mach_port_release_right(mach_port_name_t port, mach_port_right_t right);

#endif // _XPC_UTIL_H_
