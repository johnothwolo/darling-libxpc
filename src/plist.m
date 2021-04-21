#import <xpc/plist.h>
#import <xpc/objects/string.h>
#import <xpc/objects/dictionary.h>
#import <xpc/objects/array.h>
#import <xpc/objects/data.h>
#import <xpc/objects/bool.h>
#import <xpc/objects/int64.h>
#import <xpc/objects/double.h>
#import <xpc/objects/date.h>
#import <xpc/objects/uuid.h>
#import <xpc/objects/null.h>
#import <xpc/private.h>
#import <xpc/util.h>

#include <time.h>
#include <math.h>

#define __DISPATCH_INDIRECT__ 1
#include <dispatch/data_private.h>

// for now, we'll only support bplist00 and XML,
// but bplist15 and bplist16 support should also be added
// (i think our CoreFoundation doesn't have those)

// note that our XML parsing is not robust or strictly compliant in any way, shape, or form, but we're not looking to do strict parsing.
// ideally, we would have libxml do the parsing for us, but unfortunately, we're part of libSystem, so that's a big ol' no.

// Like strncmp, but the length restriction applies only to the haystack.
// Therefore, the needle *must* be null-terminated.
// Why does this exist? It's best explained with an example.
//
// Given the following arguments:
//  * haystack = char[] { 'f', 'o', 'o'} // not null terminated
//  * needle   = "foobar"                // is null terminated
//  * length   = 3
//
// `strncmp` would return 0 because it only considers the first 3 characters of both strings.
// However, `strcmp_with_length_check` would see that the haystack is shorter than the needle and return -1
static int strcmp_with_length_check(const char* haystack, const char* needle, size_t maximum_haystack_length) {
	size_t needle_length = strlen(needle);
	if (strnlen(haystack, maximum_haystack_length) < needle_length) {
		// haystack is shorter than needle
		return -1;
	}
	return strncmp(haystack, needle, needle_length);
};

static bool xpc_xml_is_valid_tag_character(char character) {
	return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') || (character >= '0' && character <= '9') || (character == '_') || (character == '-') || (character == '.');
};

xpc_plist_xml_element_type_t xpc_plist_xml_element_type_from_tag_name(const char* tag_name_start, size_t max_length) {
	xpc_plist_xml_element_type_t type = xpc_plist_xml_element_type_invalid;

	if (strcmp_with_length_check(tag_name_start, "plist", max_length) == 0) {
		type = xpc_plist_xml_element_type_plist;
	} else if (strcmp_with_length_check(tag_name_start, "array", max_length) == 0) {
		type = xpc_plist_xml_element_type_array;
	} else if (strcmp_with_length_check(tag_name_start, "data", max_length) == 0) {
		type = xpc_plist_xml_element_type_data;
	} else if (strcmp_with_length_check(tag_name_start, "date", max_length) == 0) {
		type = xpc_plist_xml_element_type_date;
	} else if (strcmp_with_length_check(tag_name_start, "dict", max_length) == 0) {
		type = xpc_plist_xml_element_type_dict;
	} else if (strcmp_with_length_check(tag_name_start, "real", max_length) == 0) {
		type = xpc_plist_xml_element_type_real;
	} else if (strcmp_with_length_check(tag_name_start, "integer", max_length) == 0) {
		type = xpc_plist_xml_element_type_integer;
	} else if (strcmp_with_length_check(tag_name_start, "string", max_length) == 0) {
		type = xpc_plist_xml_element_type_string;
	} else if (strcmp_with_length_check(tag_name_start, "true", max_length) == 0) {
		type = xpc_plist_xml_element_type_true;
	} else if (strcmp_with_length_check(tag_name_start, "false", max_length) == 0) {
		type = xpc_plist_xml_element_type_false;
	} else if (strcmp_with_length_check(tag_name_start, "key", max_length) == 0) {
		type = xpc_plist_xml_element_type_key;
	}

	max_length -= xpc_plist_xml_element_type_length(type);
	tag_name_start += xpc_plist_xml_element_type_length(type);

	// make sure the name above is a whole word (e.g. we don't want to match on "dictionary", "databank", "realizer", etc.)
	if (max_length > 0 && xpc_xml_is_valid_tag_character(tag_name_start[0])) {
		type = xpc_plist_xml_element_type_invalid;
	}

	return type;
};

size_t xpc_plist_xml_element_type_length(xpc_plist_xml_element_type_t type) {
	switch (type) {
		case xpc_plist_xml_element_type_plist:
			return 5;
		case xpc_plist_xml_element_type_array:
			return 5;
		case xpc_plist_xml_element_type_data:
			return 4;
		case xpc_plist_xml_element_type_date:
			return 4;
		case xpc_plist_xml_element_type_dict:
			return 4;
		case xpc_plist_xml_element_type_real:
			return 4;
		case xpc_plist_xml_element_type_integer:
			return 7;
		case xpc_plist_xml_element_type_string:
			return 6;
		case xpc_plist_xml_element_type_true:
			return 4;
		case xpc_plist_xml_element_type_false:
			return 5;
		case xpc_plist_xml_element_type_key:
			return 3;

		default:
			return 0;
	};
};

static bool xpc_xml_is_whitespace(char character) {
	return character == ' ' || character == '\t' || character == '\n' || character == '\r';
};

// strnstr, but automatically writes out the new length
static const char* strnstr_with_length(const char* haystack, const char* needle, size_t* length) {
	const char* result = strnstr(haystack, needle, *length);
	if (result) {
		*length -= result - haystack;
	}
	return result;
};

// memchr, but automatically writes out the new length
static const void* memchr_with_length(const void* haystack, char needle, size_t* length) {
	const void* result = memchr(haystack, needle, *length);
	if (result) {
		*length -= result - haystack;
	}
	return result;
};

// whitespace and comments are considered to be useless (outside of tags where whitespace is significant)
static bool xpc_xml_skip_useless(const char** string, size_t* max_len) {
	while (*max_len > 0) {
		if (*max_len > 4 && (*string)[0] == '<' && (*string)[1] == '!' && (*string)[2] == '-' && (*string)[3] == '-') {
			// "<!--" means we've got a comment
			*max_len -= 4;
			*string += 4;

			*string = strnstr_with_length(*string, "--", max_len);
			if (!*string) {
				// unterminated comment
				return false;
			}

			if (*max_len < 3 || (*string)[2] != '>') {
				// comments cannot contain "--" anywhere else but at the end
				return false;
			}

			*max_len -= 3;
			*string += 3;

			continue;
		} else if (!xpc_xml_is_whitespace((*string)[0])) {
			// not a comment and not whitespace? we've hit something interesting
			break;
		}

		--*max_len;
		++*string;
	}
	return true;
};

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(plist_xml_element)

XPC_CLASS_HEADER(plist_xml_element);

- (void)dealloc
{
	XPC_THIS_DECL(plist_xml_element);

	[this->object release];
	[this->cache release];

	[super dealloc];
}

- (BOOL)expectsChildren
{
	XPC_THIS_DECL(plist_xml_element);
	return this->type == xpc_plist_xml_element_type_plist || this->type == xpc_plist_xml_element_type_dict || this->type == xpc_plist_xml_element_type_array;
}

- (XPC_CLASS(plist_xml_element)*)parent
{
	XPC_THIS_DECL(plist_xml_element);
	return this->parent;
}

- (void)setParent: (XPC_CLASS(plist_xml_element)*)parent
{
	XPC_THIS_DECL(plist_xml_element);
	this->parent = parent;
}

- (XPC_CLASS(string)*)cache
{
	XPC_THIS_DECL(plist_xml_element);
	return this->cache;
}

- (void)setCache: (XPC_CLASS(string)*)cache
{
	XPC_THIS_DECL(plist_xml_element);
	[this->cache release];
	this->cache = [cache retain];
}

- (XPC_CLASS(object)*)object
{
	XPC_THIS_DECL(plist_xml_element);
	return this->object;
}

- (xpc_plist_xml_element_type_t)type
{
	XPC_THIS_DECL(plist_xml_element);
	return this->type;
}

- (instancetype)initWithType: (xpc_plist_xml_element_type_t)type
{
	if (self = [super init]) {
		XPC_THIS_DECL(plist_xml_element);

		this->type = type;

		switch (this->type) {
			case xpc_plist_xml_element_type_invalid: {
				[self release];
				return nil;
			} break;

			case xpc_plist_xml_element_type_plist: {
				// plists just take on the value of their child
			} break;

			case xpc_plist_xml_element_type_dict: {
				this->object = [XPC_CLASS(dictionary) new];
			} break;

			case xpc_plist_xml_element_type_array: {
				this->object = [XPC_CLASS(array) new];
			} break;

			default: {
				// everything else requires a string to keep track of content until they're finalized
				this->object = [XPC_CLASS(string) new];
			} break;
		}
	}
	return self;
}

- (void)finalize
{
	XPC_THIS_DECL(plist_xml_element);

	switch (this->type) {
		case xpc_plist_xml_element_type_data: {
			XPC_CLASS(string)* string = XPC_CAST(string, this->object);
			dispatch_data_t temp = NULL;
			dispatch_data_t dispatchData = NULL;
			XPC_CLASS(data)* xpcData = nil;

			temp = dispatch_data_create(string.UTF8String, string.byteLength, NULL, DISPATCH_DATA_DESTRUCTOR_NONE);
			if (!temp) {
				goto data_error;
			}

			dispatchData = dispatch_data_create_with_transform(temp, DISPATCH_DATA_FORMAT_TYPE_BASE64, DISPATCH_DATA_FORMAT_TYPE_NONE);
			if (!dispatchData) {
				goto data_error;
			}

			xpcData = [[XPC_CLASS(data) alloc] initWithDispatchData: dispatchData];
			if (!xpcData) {
				goto data_error;
			}

			[this->object release];
			this->object = [xpcData retain];

		data_out:
			[temp release];
			[dispatchData release];
			[xpcData release];
			goto handover_to_parent;

		data_error:
			[temp release];
			[dispatchData release];
			[xpcData release];
			return;
		} break;

		case xpc_plist_xml_element_type_date: {
			XPC_CLASS(string)* string = XPC_CAST(string, this->object);
			struct tm time = {
				.tm_isdst = -1,
				.tm_mday = 1,
			};

			// not using strptime because that aborts the whole thing if parts are missing;
			// we need to be able to optionally omit components (which sscanf supports)
			sscanf(string.UTF8String ? string.UTF8String : "", "%d-%d-%dT%d:%d:%dZ", &time.tm_year, &time.tm_mon, &time.tm_mday, &time.tm_hour, &time.tm_min, &time.tm_sec);
			if (time.tm_year > 0) {
				time.tm_year -= 1900;
			}
			if (time.tm_mon > 0) {
				time.tm_mon -= 1;
			}

			[this->object release];
			this->object = [[XPC_CLASS(date) alloc] initWithValue: timegm(&time) * NSEC_PER_SEC];
		} break;

		case xpc_plist_xml_element_type_real: {
			XPC_CLASS(string)* string = XPC_CAST(string, this->object);
			double value = atof(string.UTF8String);
			[this->object release];
			this->object = [[XPC_CLASS(double) alloc] initWithValue: value];
		} break;

		case xpc_plist_xml_element_type_integer: {
			XPC_CLASS(string)* string = XPC_CAST(string, this->object);
			int64_t value = atoll(string.UTF8String);
			[this->object release];
			this->object = [[XPC_CLASS(int64) alloc] initWithValue: value];
		} break;

		// no need to do any processing for strings
		//case xpc_plist_xml_element_type_string: {} break;

		case xpc_plist_xml_element_type_true: {
			[this->object release];
			this->object = [XPC_CLASS(bool) boolForValue: true];
		} break;

		case xpc_plist_xml_element_type_false: {
			[this->object release];
			this->object = [XPC_CLASS(bool) boolForValue: false];
		} break;

		// keys also don't need processing
		// (at least i think so; not sure if we need to skip leading and trailing whitespace)
		//case xpc_plist_xml_element_type_key: {} break;

		default:
			break;
	}

handover_to_parent:
	[self.parent processChild: self];
}

- (void)processChild: (XPC_CLASS(plist_xml_element)*)child
{
	XPC_THIS_DECL(plist_xml_element);
	switch (this->type) {
		case xpc_plist_xml_element_type_plist: {
			xpc_assert(!this->object);
			this->object = [child.object retain];
		} break;

		case xpc_plist_xml_element_type_array: {
			[XPC_CAST(array, this->object) addObject: child.object];
		} break;

		case xpc_plist_xml_element_type_dict: {
			if (child.type == xpc_plist_xml_element_type_key) {
				xpc_assert(!self.cache);
				xpc_assert(XPC_CHECK(string, child.object));
				self.cache = XPC_CAST(string, child.object);
			} else {
				xpc_assert(self.cache);
				[XPC_CAST(dictionary, this->object) setObject: child.object forKey: self.cache.UTF8String];
				self.cache = nil;
			}
		} break;

		default: {
			// should never happen
			xpc_assert(false);
		} break;
	}
}

+ (BOOL)pushElement: (XPC_CLASS(plist_xml_element)*)element toStack: (XPC_CLASS(plist_xml_element)**)stack
{
	// if the current top doesn't expect children, we can't push a child onto it
	if (*stack && !(*stack).expectsChildren) {
		return NO;
	}

	element.parent = *stack;

	*stack = [element retain];

	return YES;
}

+ (BOOL)popElementOfType: (xpc_plist_xml_element_type_t)expectedType fromStack: (XPC_CLASS(plist_xml_element)**)stack
{
	XPC_CLASS(plist_xml_element)* oldElement = *stack;

	if (!oldElement) {
		return NO;
	}

	if (expectedType != xpc_plist_xml_element_type_invalid && oldElement.type != expectedType) {
		return NO;
	}

	[oldElement finalize];

	*stack = oldElement.parent;
	[oldElement release];

	return YES;
}

+ (void)unwindStack: (XPC_CLASS(plist_xml_element)**)stack
{
	while (*stack) {
		XPC_CLASS(plist_xml_element)* oldElement = *stack;
		*stack = oldElement.parent;
		[oldElement release];
	}
}

@end

static xpc_object_t xpc_create_from_plist_xml(const void* _data, size_t length) {
	const char* data = _data;
	const char* root = strnstr_with_length(data, "<plist", &length);
	const char* curr = root;
	XPC_CLASS(plist_xml_element)* plistElm = nil;
	XPC_CLASS(plist_xml_element)* stack = NULL;
	XPC_CLASS(object)* result = nil;

	if (!root) {
		goto error_out;
	}

	curr = memchr_with_length(curr, '>', &length);
	if (!curr) {
		goto error_out;
	}

	--length;
	++curr;

	plistElm = [[XPC_CLASS(plist_xml_element) alloc] initWithType: xpc_plist_xml_element_type_plist];
	[XPC_CLASS(plist_xml_element) pushElement: plistElm toStack: &stack];

	while (length > 0 && stack) {
		if (!xpc_xml_skip_useless(&curr, &length)) {
			// someone left a comment unterminated
			goto error_out;
		}

		if (length > 2 && curr[0] == '<' && curr[1] == '/') {
			xpc_plist_xml_element_type_t type = xpc_plist_xml_element_type_invalid;

			length -= 2;
			curr += 2;

			type = xpc_plist_xml_element_type_from_tag_name(curr, length);
			if (type == xpc_plist_xml_element_type_invalid) {
				// invalid '</'
				goto error_out;
			}

			length -= xpc_plist_xml_element_type_length(type);
			curr += xpc_plist_xml_element_type_length(type);

			// skip to the end
			curr = memchr_with_length(curr, '>', &length);
			if (!curr) {
				goto error_out;
			}

			--length;
			++curr;

			if (![XPC_CLASS(plist_xml_element) popElementOfType: type fromStack: &stack]) {
				// invalid closing tag
				// (someone probably closed a parent before closing the child or something like that)
				goto error_out;
			}
		} else if (length > 1 && curr[0] == '<') {
			XPC_CLASS(plist_xml_element)* new_element = nil;
			xpc_plist_xml_element_type_t type = xpc_plist_xml_element_type_invalid;

			--length;
			++curr;

			type = xpc_plist_xml_element_type_from_tag_name(curr, length);
			if (type == xpc_plist_xml_element_type_invalid) {
				// invalid '<'
				goto error_out;
			}

			length -= xpc_plist_xml_element_type_length(type);
			curr += xpc_plist_xml_element_type_length(type);

			// skip attributes, if present
			// NOTE: this is technically incorrect because it is valid for '>' to appear within attribute values,
			//       but like i said before, this'll do for now; we're not looking to do robust parsing.
			curr = memchr_with_length(curr, '>', &length);
			if (!curr) {
				goto error_out;
			}

			--length;
			++curr;

			new_element = [[XPC_CLASS(plist_xml_element) alloc] initWithType: type];
			if (![XPC_CLASS(plist_xml_element) pushElement: new_element toStack: &stack]) {
				// someone tried to nest a tag where they can't (e.g. inside a <real> or a <string> or something like that)
				[new_element release];
				goto error_out;
			}
			[new_element release];

			if (curr[-2] == '/') {
				// if the character right before the '>' is '/', this is an empty element
				[XPC_CLASS(plist_xml_element) popElementOfType: type fromStack: &stack];
			}
		}

		if (stack.expectsChildren) {
			// if we expect children, we don't expect regular content, so skip straight to the important stuff
			if (!xpc_xml_skip_useless(&curr, &length)) {
				// invalid '<'
				goto error_out;
			}
			if (length > 0 && curr[0] != '<') {
				// we don't expect regular content
				goto error_out;
			}
		} else {
			// otherwise, we don't expect children, so parse everything except '<' as regular content
			const char* fragment_start = curr;
			size_t fragment_length = 0;
			while (length > 0) {
				if (curr[0] == '<') {
					if (fragment_length > 0) {
						[XPC_CAST(string, stack.object) appendString: fragment_start length: fragment_length];
						fragment_start = curr;
						fragment_length = 0;
					}

					if (strcmp_with_length_check(curr, "<!--", length) == 0) {
						length -= 4;
						curr += 4;

						curr = strnstr_with_length(curr, "-->", &length);
						if (!curr) {
							// unterminated comment
							goto error_out;
						}

						length -= 3;
						curr += 3;

						fragment_start = curr;
						fragment_length = 0;
					} else if (strcmp_with_length_check(curr, "<![CDATA[", length) == 0) {
						length -= 9;
						curr += 9;

						fragment_start = curr;
						fragment_length = 0;

						curr = strnstr_with_length(curr, "]]>", &length);
						if (!curr) {
							// unterminated CDATA
							goto error_out;
						}

						fragment_length = curr - fragment_start;

						[XPC_CAST(string, stack.object) appendString: fragment_start length: fragment_length];

						length -= 3;
						curr += 3;

						fragment_start = curr;
						fragment_length = 0;
					} else if (strcmp_with_length_check(curr, "</", length) == 0) {
						// the tag is closing
						// let's go back to the outer loop and allow it to process it
						break;
					} else {
						// invalid '<' (since we don't expect children)
						goto error_out;
					}
				} else if (curr[0] == '&') {
					// ampersands are special
					char character_to_append = '&';

					if (fragment_length > 0) {
						[XPC_CAST(string, stack.object) appendString: fragment_start length: fragment_length];
						fragment_start = curr;
						fragment_length = 0;
					}

					if (strcmp_with_length_check(curr, "&amp;", length) == 0) {
						character_to_append = '&';
						length -= 5;
						curr += 5;
					} else if (strcmp_with_length_check(curr, "&lt;", length) == 0) {
						character_to_append = '<';
						length -= 4;
						curr += 4;
					} else if (strcmp_with_length_check(curr, "&gt;", length) == 0) {
						character_to_append = '>';
						length -= 4;
						curr += 4;
					} else if (strcmp_with_length_check(curr, "&quot;", length) == 0) {
						character_to_append = '"';
						length -= 6;
						curr += 6;
					} else if (strcmp_with_length_check(curr, "&apos;", length) == 0) {
						character_to_append = '\'';
						length -= 6;
						curr += 6;
					} else {
						// regular '&'
						// invalid, but let's accept it as a plain ampersand
						character_to_append = '&';
						length -= 1;
						curr += 1;
					}

					[XPC_CAST(string, stack.object) appendString: &character_to_append length: 1];

					fragment_start = curr;
					fragment_length = 0;
				} else {
					// everything else is regular content
					++fragment_length;
					--length;
					++curr;
				}
			}
		}
	}

	if (stack) {
		// end of content but still have elements that need to be closed
		goto error_out;
	}

	result = [plistElm.object retain];
	[plistElm release];

	return result;

error_out:
	[XPC_CLASS(plist_xml_element) unwindStack: &stack];
	[plistElm release];
	return NULL;
};

OS_OBJECT_NONLAZY_CLASS
@implementation XPC_CLASS(plist_binary_v0_deserializer)

XPC_CLASS_HEADER(plist_binary_v0_deserializer);

- (void)dealloc
{
	XPC_THIS_DECL(plist_binary_v0_deserializer);

	[this->root_object release];

	[super dealloc];
}

- (XPC_CLASS(object)*)rootObject
{
	XPC_THIS_DECL(plist_binary_v0_deserializer);
	return this->root_object;
}

- (instancetype)initWithData: (const void*)data length: (NSUInteger)length
{
	if (self = [super init]) {
		XPC_THIS_DECL(plist_binary_v0_deserializer);
		const xpc_plist_binary_v0_trailer_t* trailer = NULL;

		this->data = data;
		this->length = length;

		// we need at least the magic, version, and trailer
		if (this->length < 40) {
			[self release];
			return nil;
		}

		trailer = (const xpc_plist_binary_v0_trailer_t*)(data + length - sizeof(xpc_plist_binary_v0_trailer_t));
		this->offset_size = trailer->offset_size;
		this->reference_size = trailer->reference_size;
		this->object_count = OSReadBigInt64(&trailer->object_count, 0);
		this->root_object_reference_number = OSReadBigInt64(&trailer->root_object_reference_number, 0);
		this->offset_table_offset = OSReadBigInt64(&trailer->offset_table_offset, 0);

		this->offset_table = this->data + this->offset_table_offset;

		this->root_object = [self readObject: this->root_object_reference_number];
	}
	return self;
}

- (NSUInteger)readOffset: (NSUInteger)referenceNumber
{
	XPC_THIS_DECL(plist_binary_v0_deserializer);
	const uint8_t* start = &this->offset_table[referenceNumber * this->offset_size];
	if (referenceNumber >= this->object_count) {
		return NSUIntegerMax;
	}
	if (this->offset_size == 1) {
		return *start;
	} else if (this->offset_size == 2) {
		return OSReadBigInt16(start, 0);
	} else if (this->offset_size == 4) {
		return OSReadBigInt32(start, 0);
	} else if (this->offset_size == 8) {
		return OSReadBigInt64(start, 0);
	} else {
		// invalid offset size
		return NSUIntegerMax;
	}
}

- (XPC_CLASS(int64)*)readInteger: (const uint8_t*)object
{
	uint8_t pow_2_bytes = object[0] & 0x0f;
	int64_t value = 0;
	if (pow_2_bytes == 0) {
		value = object[1];
	} else if (pow_2_bytes == 1) {
		value = OSReadBigInt16(&object[1], 0);
	} else if (pow_2_bytes == 2) {
		value = OSReadBigInt32(&object[1], 0);
	} else if (pow_2_bytes == 3) {
		value = OSReadBigInt64(&object[1], 0);
	} else {
		// invalid length
		return nil;
	}
	return [[XPC_CLASS(int64) alloc] initWithValue: value];
}

- (XPC_CLASS(double)*)readReal: (const uint8_t*)object
{
	uint8_t pow_2_bytes = object[0] & 0x0f;
	double value = 0;
	if (pow_2_bytes == 2) {
		// yes, yes, i know this is hacky;
		// but CF is doing the same thing so ¯\_(ツ)_/¯
		uint32_t as_uint;
		memcpy(&as_uint, &object[1], 4);
		as_uint = OSSwapBigToHostInt32(as_uint);
		value = *(float*)&as_uint;
	} else if (pow_2_bytes == 3) {
		// ditto
		uint64_t as_uint;
		memcpy(&as_uint, &object[1], 8);
		as_uint = OSSwapBigToHostInt64(as_uint);
		value = *(double*)&as_uint;
	} else {
		// invalid length
		return nil;
	}
	return [[XPC_CLASS(double) alloc] initWithValue: value];
}

- (XPC_CLASS(date)*)readDate: (const uint8_t*)object
{
	double value = 0;
	uint64_t as_uint;
	memcpy(&as_uint, &object[1], 8);
	as_uint = OSSwapBigToHostInt64(as_uint);
	value = *(double*)&as_uint;
	return [[XPC_CLASS(date) alloc] initWithAbsoluteValue: value];
}

- (NSUInteger)readLength: (const uint8_t*)object dataStart: (const uint8_t**)dataStart
{
	uint8_t marker = object[0] & 0x0f;
	if (marker == 0x0f) {
		uint8_t pow_2_bytes = object[1] & 0x0f;
		NSUInteger result = 0;
		if (pow_2_bytes == 0) {
			result = object[2];
			*dataStart = &object[3];
		} else if (pow_2_bytes == 1) {
			result = OSReadBigInt16(&object[2], 0);
			*dataStart = &object[4];
		} else if (pow_2_bytes == 2) {
			result = OSReadBigInt32(&object[2], 0);
			*dataStart = &object[6];
		} else if (pow_2_bytes == 3) {
			result = OSReadBigInt64(&object[2], 0);
			*dataStart = &object[10];
		} else {
			// invalid length
			return NSUIntegerMax;
		}
		return result;
	} else {
		*dataStart = &object[1];
		return marker;
	}
}

- (XPC_CLASS(data)*)readData: (const uint8_t*)object
{
	const uint8_t* bytes = NULL;
	NSUInteger length = [self readLength: object dataStart: &bytes];
	if (length == NSUIntegerMax) {
		return nil;
	}
	return [[XPC_CLASS(data) alloc] initWithBytes: bytes length: length];
}

- (XPC_CLASS(string)*)readASCIIString: (const uint8_t*)object
{
	const uint8_t* bytes = NULL;
	NSUInteger length = [self readLength: object dataStart: &bytes];
	if (length == NSUIntegerMax) {
		return nil;
	}
	return [[XPC_CLASS(string) alloc] initWithUTF8String: (const char*)bytes byteLength: length];
}

- (XPC_CLASS(string)*)readUTF16String: (const uint8_t*)object
{
	const uint8_t* bytes = NULL;
	NSUInteger length = [self readLength: object dataStart: &bytes];
	dispatch_data_t data = NULL;
	dispatch_data_t transformed = NULL;
	XPC_CLASS(string)* result = nil;

	if (length == NSUIntegerMax) {
		return nil;
	}

	data = dispatch_data_create(bytes, length, NULL, DISPATCH_DATA_DESTRUCTOR_NONE);
	// how nice of them to have something specifically for this purpose :)
	transformed = dispatch_data_create_with_transform(data, DISPATCH_DATA_FORMAT_TYPE_UTF16BE, DISPATCH_DATA_FORMAT_TYPE_UTF8);
	result = [[XPC_CLASS(string) alloc] initWithUTF8String: dispatch_data_get_flattened_bytes_4libxpc(transformed) byteLength: dispatch_data_get_size(transformed)];
	[transformed release];
	[data release];

	return result;
}

- (XPC_CLASS(uuid)*)readUUID: (const uint8_t*)object
{
	uint8_t length = (object[0] & 0x0f) + 1;
	uuid_t bytes;
	memcpy(bytes, &object[1], length);
	return [[XPC_CLASS(uuid) alloc] initWithBytes: bytes];
}

- (NSUInteger)readReferenceNumber: (const uint8_t*)start next: (const uint8_t**)next
{
	XPC_THIS_DECL(plist_binary_v0_deserializer);
	uint64_t result = 0;
	if (this->reference_size == 1) {
		result = *start;
	} else if (this->reference_size == 2) {
		result = OSReadBigInt16(start, 0);
	} else if (this->reference_size == 4) {
		result = OSReadBigInt32(start, 0);
	} else if (this->reference_size == 8) {
		result = OSReadBigInt64(start, 0);
	} else {
		// invalid offset size
		result = NSUIntegerMax;
	}
	if (next) {
		*next = start + this->reference_size;
	}
	return result;
}

- (XPC_CLASS(array)*)readArray: (const uint8_t*)object
{
	const uint8_t* bytes = NULL;
	NSUInteger count = [self readLength: object dataStart: &bytes];
	XPC_CLASS(array)* result = nil;

	if (count == NSUIntegerMax) {
		return nil;
	}

	result = [XPC_CLASS(array) new];

	for (NSUInteger i = 0; i < count; ++i) {
		NSUInteger referenceNumber = [self readReferenceNumber: bytes next: &bytes];
		XPC_CLASS(object)* object = [self readObject: referenceNumber];
		[result addObject: object];
		[object release];
	}

	return result;
}

- (XPC_CLASS(array)*)readSet: (const uint8_t*)object
{
	return [self readArray: object];
}

- (XPC_CLASS(dictionary)*)readDictionary: (const uint8_t*)object
{
	XPC_THIS_DECL(plist_binary_v0_deserializer);
	const uint8_t* key_bytes = NULL;
	const uint8_t* value_bytes = NULL;
	NSUInteger count = [self readLength: object dataStart: &key_bytes];
	XPC_CLASS(dictionary)* result = nil;

	if (count == NSUIntegerMax) {
		return nil;
	}

	value_bytes = key_bytes + (this->reference_size * count);
	result = [XPC_CLASS(dictionary) new];

	for (NSUInteger i = 0; i < count; ++i) {
		NSUInteger keyReferenceNumber = [self readReferenceNumber: key_bytes next: &key_bytes];
		NSUInteger valueReferenceNumber = [self readReferenceNumber: value_bytes next: &value_bytes];
		XPC_CLASS(object)* key = [self readObject: keyReferenceNumber];
		XPC_CLASS(object)* value = [self readObject: valueReferenceNumber];
		TO_OBJC_CHECKED(string, key, keyString) {
			[result setObject: value forKey: keyString.UTF8String];
		}
		[key release];
		[value release];
	}

	return result;
}

- (XPC_CLASS(object)*)readObject: (NSUInteger)referenceNumber
{
	XPC_THIS_DECL(plist_binary_v0_deserializer);
	NSUInteger offset = [self readOffset: referenceNumber];
	const uint8_t* objectStart = &this->data[offset];
	XPC_CLASS(object)* result = nil;
	xpc_plist_binary_v0_object_type_t type;

	if (offset == NSUIntegerMax) {
		goto error_out;
	}

	type = objectStart[0] >> 4;

	switch (type) {
		case xpc_plist_binary_v0_object_type_singleton: {
			uint8_t identifier = objectStart[0] & 0x0f;
			if (identifier == 0) {
				// null
				result = [XPC_CLASS(null) null];
			} else if (identifier == 8 || identifier == 9) {
				// boolean
				result = [XPC_CLASS(bool) boolForValue: identifier == 9];
			} else {
				// unknown/unexpected
				result = nil;
			}
		} break;

		case xpc_plist_binary_v0_object_type_integer: {
			result = [self readInteger: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_real: {
			result = [self readReal: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_date: {
			result = [self readDate: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_data: {
			result = [self readData: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_ascii_string: {
			result = [self readASCIIString: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_utf16_string: {
			result = [self readUTF16String: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_uuid: {
			result = [self readUUID: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_array: {
			result = [self readArray: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_set: {
			result = [self readSet: objectStart];
		} break;

		case xpc_plist_binary_v0_object_type_dictionary: {
			result = [self readDictionary: objectStart];
		} break;
	}

	return result;

error_out:
	[result release];
	return nil;
}

@end

static xpc_object_t xpc_create_from_plist_binary_v0(const void* data, size_t length) {
	XPC_CLASS(plist_binary_v0_deserializer)* deserialier = [[XPC_CLASS(plist_binary_v0_deserializer) alloc] initWithData: data length: length];
	XPC_CLASS(object)* result = nil;

	if (!deserialier) {
		goto error_out;
	}

	result = [deserialier.rootObject retain];
	[deserialier release];

	return result;

error_out:
	[deserialier release];
	return NULL;
};

static xpc_object_t xpc_create_from_plist_binary_v1(const void* _data, size_t length) {
	const char* data = _data;
	xpc_stub();
	return NULL;
};

XPC_EXPORT
xpc_object_t xpc_create_from_plist(const void* _data, size_t length) {
	const char* data = _data;
	xpc_object_t result = NULL;

	// XML plists are surely longer than 8 bytes and binary plists are, too
	if (length < 8) {
		goto out;
	}

	@autoreleasepool {
		if (strcmp_with_length_check(data, "bplist", length) == 0) {
			if (data[6] == '0') {
				// version 0
				result = xpc_create_from_plist_binary_v0(data, length);
			} else if (data[6] == '1') {
				// version 1
				result = xpc_create_from_plist_binary_v1(data, length);
			} else {
				// unknown version
				// leave result as NULL
			}
		} else {
			result = xpc_create_from_plist_xml(data, length);
		}
	}

out:
	return result;
};

XPC_EXPORT
void xpc_create_from_plist_descriptor(int fd, dispatch_queue_t queue, void(^callback)(xpc_object_t result)) {
	dispatch_read(fd, SIZE_MAX, queue, ^(dispatch_data_t data, int error) {
		if (error == 0) {
			xpc_object_t result = xpc_create_from_plist(dispatch_data_get_flattened_bytes_4libxpc(data), dispatch_data_get_size(data));
			callback(result);
			[result release];
		} else {
			callback(NULL);
		}
	});
};
