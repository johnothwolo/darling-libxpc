#ifndef XPC_PRIVATE_H_
#define XPC_PRIVATE_H_

#include <uuid/uuid.h>
#include <xpc/xpc.h>

// `notify_client.c` includes `xpc/private.h` and expects it to define `xpc_copy_entitlement_for_token`, which we have in `launchd.h`
#include <xpc/launchd.h>
#include <xpc/private/pipe.h>
#include <xpc/private/endpoint.h>
#include <xpc/private/mach_send.h>
#include <xpc/private/mach_recv.h>

#ifdef __cplusplus
extern "C" {
#endif


XPC_EXPORT
XPC_TYPE(_xpc_type_mach_send);
#define XPC_TYPE_MACH_SEND (&_xpc_type_mach_send)

XPC_EXPORT
XPC_TYPE(_xpc_type_mach_recv);
#define XPC_TYPE_MACH_RECV (&_xpc_type_mach_recv)

XPC_EXPORT
XPC_TYPE(_xpc_type_serializer);
#define XPC_TYPE_SERIALIZER (&_xpc_type_serializer)

XPC_EXPORT
XPC_TYPE(_xpc_type_pipe);
#define XPC_TYPE_PIPE (&_xpc_type_pipe)

XPC_EXPORT
XPC_TYPE(_xpc_type_bundle);
#define XPC_TYPE_BUNDLE (&_xpc_type_bundle)

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

xpc_object_t _od_rpc_call(const char *procname, xpc_object_t payload, xpc_pipe_t (*get_pipe)(bool));

xpc_object_t xpc_create_with_format(const char * format, ...);

xpc_object_t xpc_create_reply_with_format(xpc_object_t original, const char * format, ...);

xpc_object_t xpc_create_from_plist(void *data, size_t size);

void xpc_dictionary_get_audit_token(xpc_object_t, audit_token_t *);

void xpc_connection_set_target_uid(xpc_connection_t connection, uid_t uid);
void xpc_connection_set_instance(xpc_connection_t connection, uuid_t uid);
void xpc_dictionary_set_mach_send(xpc_object_t object, const char* key, mach_port_t port);

xpc_object_t xpc_connection_copy_entitlement_value(xpc_connection_t connection, const char* entitlement);

void xpc_transaction_exit_clean();

void _xpc_string_set_value(xpc_object_t xstring, const char* new_string);

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

mach_port_t _xpc_dictionary_extract_mach_send(xpc_object_t xdict, const char* key);

mach_port_t xpc_dictionary_copy_mach_send(xpc_object_t xdict, const char* key);

mach_port_t xpc_dictionary_extract_mach_recv(xpc_object_t xdict, const char* key);

xpc_object_t xpc_dictionary_get_array(xpc_object_t xdict, const char* key);

xpc_connection_t xpc_dictionary_get_connection(xpc_object_t xdict);

void _xpc_data_set_value(xpc_object_t xdata, const void* bytes, size_t length);

size_t xpc_data_get_inline_max(xpc_object_t xdata);

xpc_object_t _xpc_bool_create_distinct(bool value);

void _xpc_bool_set_value(xpc_object_t xbool, bool value);

const char* xpc_type_get_name(xpc_type_t xtype);

xpc_connection_t xpc_connection_create_listener(const char* name, dispatch_queue_t queue);

#ifdef __cplusplus
}
#endif

#endif

