/**
 * This file is part of Darling.
 *
 * Copyright (C) 2021 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _XPC_OBJECTS_PLIST_XML_ELEMENT_H_
#define _XPC_OBJECTS_PLIST_XML_ELEMENT_H_

#import <xpc/objects/base.h>

// NOTE: this class is not present in the official libxpc

// we have this class to help with XML plist parsing

XPC_CLASS_DECL(plist_xml_element);

@class XPC_CLASS(plist_xml_element);
@class XPC_CLASS(string);

OS_ENUM(xpc_plist_xml_element_type, uint8_t,
	xpc_plist_xml_element_type_invalid,
	xpc_plist_xml_element_type_plist,
	xpc_plist_xml_element_type_array,
	xpc_plist_xml_element_type_data,
	xpc_plist_xml_element_type_date,
	xpc_plist_xml_element_type_dict,
	xpc_plist_xml_element_type_real,
	xpc_plist_xml_element_type_integer,
	xpc_plist_xml_element_type_string,
	xpc_plist_xml_element_type_true,
	xpc_plist_xml_element_type_false,
	xpc_plist_xml_element_type_key,
);

xpc_plist_xml_element_type_t xpc_plist_xml_element_type_from_tag_name(const char* tag_name_start, size_t max_length);
size_t xpc_plist_xml_element_type_length(xpc_plist_xml_element_type_t type);

struct xpc_plist_xml_element_s {
	struct xpc_object_s base;
	XPC_CLASS(plist_xml_element)* parent;
	XPC_CLASS(object)* object; // for elements that don't expect children, this is a string with the inner text of the element
	XPC_CLASS(string)* cache; // for dictionaries only; to store the key while waiting for its value
	xpc_plist_xml_element_type_t type;
};

@interface XPC_CLASS_INTERFACE(plist_xml_element)

@property(readonly) BOOL expectsChildren;
@property(assign) XPC_CLASS(plist_xml_element)* parent;
@property(readonly) XPC_CLASS(object)* object;
@property(strong) XPC_CLASS(string)* cache;
@property(readonly) xpc_plist_xml_element_type_t type;

- (instancetype)initWithType: (xpc_plist_xml_element_type_t)type;

- (void)finalize;

/**
 * Called by child elements when they're finalized.
 */
- (void)processChild: (XPC_CLASS(plist_xml_element)*)child;

// these next methods are used to manage an XML parsing stack

+ (BOOL)pushElement: (XPC_CLASS(plist_xml_element)*)element toStack: (XPC_CLASS(plist_xml_element)**)stack;
// will automatically finalize the element on the top of the stack
+ (BOOL)popElementOfType: (xpc_plist_xml_element_type_t)expectedType fromStack: (XPC_CLASS(plist_xml_element)**)stack;
+ (void)unwindStack: (XPC_CLASS(plist_xml_element)**)stack;

@end

#endif // _XPC_OBJECTS_PLIST_XML_ELEMENT_H_
