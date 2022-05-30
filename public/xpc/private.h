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

#ifndef XPC_PRIVATE_H_
#define XPC_PRIVATE_H_

#include <os/lock.h>
#include <uuid/uuid.h>
#include <xpc/xpc.h>

// `notify_client.c` includes `xpc/private.h` and expects it to define `xpc_copy_entitlement_for_token`, which we have in `launchd.h`
#include <xpc/launchd.h>
#include <xpc/private/pipe.h>
#include <xpc/private/endpoint.h>
#include <xpc/private/mach_send.h>
#include <xpc/private/mach_recv.h>
#include <xpc/private/date.h>
#include <xpc/private/plist.h>
#include <xpc/private/bundle.h>

__BEGIN_DECLS

XPC_EXPORT
XPC_TYPE(_xpc_type_serializer);
#define XPC_TYPE_SERIALIZER (&_xpc_type_serializer)

XPC_EXPORT
XPC_TYPE(_xpc_type_service);
#define XPC_TYPE_SERVICE (&_xpc_type_service)

XPC_EXPORT
XPC_TYPE(_xpc_type_service_instance);
#define XPC_TYPE_SERVICE_INSTANCE (&_xpc_type_service_instance)

XPC_EXPORT
XPC_TYPE(_xpc_type_file_transfer);
#define XPC_TYPE_FILE_TRANSFER (&_xpc_type_file_transfer)

int _xpc_runtime_is_app_sandboxed();

xpc_object_t xpc_create_with_format(const char * format, ...);

xpc_object_t xpc_create_reply_with_format(xpc_object_t original, const char * format, ...);

xpc_object_t xpc_create_from_plist(const void *data, size_t size);

void xpc_dictionary_get_audit_token(xpc_object_t, audit_token_t *);

void xpc_connection_set_target_uid(xpc_connection_t connection, uid_t uid);
void xpc_connection_set_instance(xpc_connection_t connection, uuid_t uid);
void xpc_dictionary_set_mach_send(xpc_object_t object, const char* key, mach_port_t port);

xpc_object_t xpc_connection_copy_entitlement_value(xpc_connection_t connection, const char* entitlement);

int xpc_domain_routine(int routine, xpc_object_t msg, xpc_object_t *resp);

void xpc_transaction_exit_clean();

void xpc_string_set_value(xpc_object_t xstring, const char* new_string);

xpc_object_t xpc_string_create_no_copy(const char* string);

typedef void (*xpc_array_applier_f)(size_t index, xpc_object_t value, void* context);

void xpc_array_apply_f(xpc_object_t xarray, void* context, xpc_array_applier_f applier);

void xpc_array_set_mach_send(xpc_object_t xarray, size_t index, mach_port_t value);

mach_port_t xpc_array_copy_mach_send(xpc_object_t xarray, size_t index);

xpc_object_t xpc_array_get_array(xpc_object_t xarray, size_t index);

xpc_object_t xpc_array_get_dictionary(xpc_object_t xarray, size_t index);

xpc_object_t _xpc_dictionary_create_reply_with_port(mach_port_t port);

mach_msg_id_t _xpc_dictionary_extract_reply_msg_id(xpc_object_t xdict);

mach_port_t _xpc_dictionary_extract_reply_port(xpc_object_t xdict);

mach_msg_id_t _xpc_dictionary_get_reply_msg_id(xpc_object_t xdict);

void _xpc_dictionary_set_remote_connection(xpc_object_t xdict, xpc_connection_t xconn);

void _xpc_dictionary_set_reply_msg_id(xpc_object_t xdict, mach_msg_id_t msg_id);

typedef void (*xpc_dictionary_applier_f)(const char* key, xpc_object_t value, void* context);

void xpc_dictionary_apply_f(xpc_object_t xdict, void* context, xpc_dictionary_applier_f applier);

char* xpc_dictionary_copy_basic_description(xpc_object_t xdict);

bool xpc_dictionary_expects_reply(xpc_object_t xdict);

void xpc_dictionary_handoff_reply(xpc_object_t xdict, dispatch_queue_t queue, dispatch_block_t block);

void xpc_dictionary_handoff_reply_f(xpc_object_t xdict, dispatch_queue_t queue, void* context, dispatch_function_t function);

void xpc_dictionary_send_reply(xpc_object_t xdict);

void xpc_dictionary_set_mach_recv(xpc_object_t xdict, const char* key, mach_port_t recv);

mach_port_t xpc_dictionary_extract_mach_send(xpc_object_t xdict, const char* key);

mach_port_t xpc_dictionary_copy_mach_send(xpc_object_t xdict, const char* key);

mach_port_t xpc_dictionary_extract_mach_recv(xpc_object_t xdict, const char* key);

xpc_object_t xpc_dictionary_get_array(xpc_object_t xdict, const char* key);

xpc_connection_t xpc_dictionary_get_connection(xpc_object_t xdict);

void xpc_data_set_value(xpc_object_t xdata, const void* bytes, size_t length);

size_t xpc_data_get_inline_max(xpc_object_t xdata);

xpc_object_t _xpc_bool_create_distinct(bool value);

/*!
* @function xpc_bool_set_value
*
* @abstract
* Sets the underlying Boolean object from the value.
*
* @param xbool
* The Boolean object which is to be examined.
*
* @param value
* The Boolean value to pass to the object.
*
* @result
* The underlying Boolean value or false if the given object was not an XPC Boolean object.
*/
bool xpc_bool_set_value(xpc_object_t xbool, bool value);

const char* xpc_type_get_name(xpc_type_t xtype);

xpc_connection_t xpc_connection_create_listener(const char* name, dispatch_queue_t queue);

void xpc_ktrace_pid1(unsigned int, uint64_t);

const char *xpc_strerror(int error);

__END_DECLS

#endif

