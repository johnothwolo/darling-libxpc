#include <xpc/private.h>
#include <stdio.h>

int _xpc_runtime_is_app_sandboxed()
{
	return 0;
}

static xpc_object_t xpc_create_with_format_impl(const char * format, va_list args) {
	// simplistic implementation for now
	//
	// everywhere this function and `xpc_create_reply_with_format` are used in Darling,
	// only simple shallow objects are created

	xpc_object_t result = NULL;

	// TODO: work with non-dictionaries
	// like i said before, this isn't a big issue because everyone using this function
	// and `xpc_create_reply_with_format` in Darling is only creating dictionaries
	if (format[0] != '{')
		return NULL;

	++format;

	result = xpc_dictionary_create(NULL, NULL, 0);

	bool inKey = true;
	bool ignoringWhitespace = true;
	const char* key_start = NULL;
	const char* key_end = NULL;
	const char* str_start = NULL;
	const char* str_end = NULL;

	for (; *format != '\0'; ++format) {
		// TODO: nested objects
		if (*format == '{') {
			if (result)
				xpc_release(result);
			result = NULL;
			break;
		}

		if (*format == '}' || (!inKey && *format == ',')) {
			inKey = true;
			ignoringWhitespace = true;

			if (!str_start) {
				// empty dictionary
				if (!key_start && *format == '}')
					break;

				// otherwise: bad format
				if (result)
					xpc_release(result);
				result = NULL;
				break;
			}

			// remove trailing whitespace
			while (*(str_end - 1) == ' ' || *(str_end - 1) == '\t')
				--str_end;

			size_t key_len = key_end - key_start;
			size_t str_len = str_end - str_start;

			// needed in order to have a null terminator
			char* key_buf[key_len + 1];
			strncpy(key_buf, key_start, key_len);
			key_buf[key_len] = '\0';

			const char* key = key_buf;

			// replace "%string" with a vararg
			if (key_len == 7 && !strncmp(key_start, "%string", key_len))
				key = va_arg(args, const char*);

			xpc_object_t value = NULL;

			// replace "%string" with a vararg
			// replace "%value" with a vararg
			if (str_len == 7 && !strncmp(str_start, "%string", str_len)) {
				value = xpc_string_create(va_arg(args, const char*));
			} else if (str_len == 6 && !strncmp(str_start, "%value", str_len)) {
				value = va_arg(args, xpc_object_t);
				xpc_retain(value); // to balance out the `xpc_release` later on
			} else {
				char* str_buf[str_len + 1];
				strncpy(str_buf, str_start, str_len);
				str_buf[str_len] = '\0';
				value = xpc_string_create(str_buf);
			}

			xpc_dictionary_set_value(result, key, value);
			xpc_release(value);

			if (*format == '}')
				break;
		} else if (inKey) {
			if (*format == ':') {
				inKey = false;
				ignoringWhitespace = true;
				continue;
			}

			if (*format == ',') {
				// bad format
				if (result)
					xpc_release(result);
				result = NULL;
				break;
			}

			if (ignoringWhitespace) {
				if (*format == ' ' || *format == '\t')
					continue;
				ignoringWhitespace = false;
			}

			if (!key_start)
				key_start = format;
			key_end = format + 1;
		} else {
			if (*format == ':') {
				// bad format
				if (result)
					xpc_release(result);
				result = NULL;
				break;
			}

			if (ignoringWhitespace) {
				if (*format == ' ' || *format == '\t')
					continue;
				ignoringWhitespace = false;
			}

			if (!str_start)
				str_start = format;
			str_end = format + 1;
		}
	}

	return result;
}

xpc_object_t xpc_create_with_format(const char * format, ...) {
	va_list args;
	va_start(args, format);
	xpc_object_t result = xpc_create_with_format_impl(format, args);
	va_end(args);
	return result;
};

xpc_object_t xpc_create_reply_with_format(xpc_object_t original, const char * format, ...) {
	xpc_object_t reply = xpc_dictionary_create_reply(original);

	if (!reply)
		return NULL;

	va_list args;
	va_start(args, format);
	xpc_object_t result = xpc_create_with_format_impl(format, args);
	va_end(args);

	// copy reply keys into result
	//
	// we actually don't need to do this ATM, because `xpc_dictionary_create_reply` just
	// creates an empty dictionary, but it's here so we're good if in the future its behavior
	// is fixed and it does add a reference to the original xpc_object
	xpc_dictionary_apply(reply, ^bool (const char* key, xpc_object_t value) {
		xpc_dictionary_set_value(result, key, value);
		return true;
	});

	xpc_release(reply);

	return result;
};

xpc_object_t xpc_connection_copy_entitlement_value(xpc_connection_t connection, const char* entitlement) {
	printf("%s\n", __PRETTY_FUNCTION__);
	return NULL;
};
