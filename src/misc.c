/*
 * Copyright 2014-2015 iXsystems, Inc.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sbuf.h>
#include <mach/mach.h>
#include <xpc/launchd.h>
#include <libkern/OSAtomic.h>
#include <assert.h>
#include <syslog.h>
#include <stdarg.h>
#include <uuid/uuid.h>
#include <stdio.h>
#include <xpc/internal.h>
#include <libkern/OSByteOrder.h>
#include <sys/sysctl.h>

#define RECV_BUFFER_SIZE	65536

#include <xpc/private.h>
#include <xpc/serialization.h>

#define sbuf_new_auto() sbuf_new(NULL, NULL, 0, SBUF_AUTOEXTEND)

#define MAX_RECV 8192
#define XPC_RECV_SIZE			\
    MAX_RECV - 				\
    sizeof(mach_msg_header_t) - 	\
    sizeof(mach_msg_trailer_t) - 	\
    sizeof(uint64_t) - 			\
    sizeof(size_t)

const char *const _xpc_event_key_name = "XPCEventName";

struct xpc_message {
	mach_msg_header_t header;
	size_t size;
	uint64_t id;
	char data[0];
	mach_msg_trailer_t trailer;
};

struct xpc_recv_message {
	mach_msg_header_t header;
	size_t size;
	uint64_t id;
	char data[XPC_RECV_SIZE];
	mach_msg_trailer_t trailer;
};

static void xpc_copy_description_level(xpc_object_t obj, struct sbuf *sbuf,
    int level);

void
fail_log(const char *exp)
{
	debugf("%s", exp);
	//sleep(1);
	printf("%s", exp);
	//abort();
}

static void
xpc_dictionary_destroy(struct xpc_object *dict)
{
	struct xpc_dict_head *head;
	struct xpc_dict_pair *p, *ptmp;

	head = &dict->xo_dict;

	TAILQ_FOREACH_SAFE(p, head, xo_link, ptmp) {
		TAILQ_REMOVE(head, p, xo_link);
		free(p->key);
		xpc_release(p->value);
		free(p);
	}
}

static void
xpc_array_destroy(struct xpc_object *dict)
{
	struct xpc_object *p, *ptmp;
	struct xpc_array_head *head;

	head = &dict->xo_array;

	TAILQ_FOREACH_SAFE(p, head, xo_link, ptmp) {
		TAILQ_REMOVE(head, p, xo_link);
		xpc_release(p);
	}
}

void
xpc_object_destroy(struct xpc_object *xo)
{
	if (xo->xo_refcnt == _XPC_KEEP_ALIVE)
		return;

	if (xo->xo_xpc_type == _XPC_TYPE_DICTIONARY)
		xpc_dictionary_destroy(xo);

	if (xo->xo_xpc_type == _XPC_TYPE_ARRAY)
		xpc_array_destroy(xo);

	if (xo->xo_xpc_type == _XPC_TYPE_CONNECTION)
		xpc_connection_destroy(xo);

	if (xo->xo_xpc_type == _XPC_TYPE_STRING)
		free(xo->xo_u.str);

	if (xo->xo_xpc_type == _XPC_TYPE_DATA)
		free(xo->xo_u.ptr);

	free(xo);
}

xpc_object_t
xpc_retain(xpc_object_t obj)
{
	struct xpc_object *xo = obj;
	if (xo->xo_refcnt == _XPC_KEEP_ALIVE)
		return obj;

	OSAtomicIncrement32(&xo->xo_refcnt);
	return (obj);
}

void
xpc_release(xpc_object_t obj)
{
	struct xpc_object *xo = obj;
	if (xo->xo_refcnt == _XPC_KEEP_ALIVE)
		return;

	if (OSAtomicDecrement32(&xo->xo_refcnt) > 0)
		return;

	xpc_object_destroy(xo);
}

// The other one is unsafe?
// This is called by Security and is private
void
xpc_release_safe(xpc_object_t obj)
{
	xpc_release(obj);
}

static const char *xpc_errors[] = {
	"No Error Found",
	"No Memory",
	"Invalid Argument",
	"No Such Process"
};


const char *
xpc_strerror(int error)
{

	if (error > EXMAX || error < 0)
		return "BAD ERROR";
	return (xpc_errors[error]);
}

char *
xpc_copy_description(xpc_object_t obj)
{
	char *result;
	struct sbuf *sbuf;

	sbuf = sbuf_new_auto();
	xpc_copy_description_level(obj, sbuf, 0);
	sbuf_finish(sbuf);
	result = strdup(sbuf_data(sbuf));
	sbuf_delete(sbuf);

	return (result);
}

static void
xpc_copy_description_level(xpc_object_t obj, struct sbuf *sbuf, int level)
{
	struct xpc_object *xo = obj;
#ifndef __APPLE__
	struct uuid *id;
#else
	uuid_t id;
#endif
	char *uuid_str;
	uint32_t uuid_status;

	if (obj == NULL) {
		sbuf_printf(sbuf, "<null value>\n");
		return;
	}

	sbuf_printf(sbuf, "(%s) ", _xpc_get_type_name(obj));

	switch (xo->xo_xpc_type) {
	case _XPC_TYPE_DICTIONARY:
		sbuf_printf(sbuf, "\n");
		xpc_dictionary_apply(xo, ^(const char *k, xpc_object_t v) {
			sbuf_printf(sbuf, "%*s\"%s\": ", level * 4, " ", k);
			xpc_copy_description_level(v, sbuf, level + 1);
			return ((bool)true);
		});
		break;

	case _XPC_TYPE_ARRAY:
		sbuf_printf(sbuf, "\n");
		xpc_array_apply(xo, ^(size_t idx, xpc_object_t v) {
			sbuf_printf(sbuf, "%*s%ld: ", level * 4, " ", idx);
			xpc_copy_description_level(v, sbuf, level + 1);
			return ((bool)true);
		});
		break;

	case _XPC_TYPE_BOOL:
		sbuf_printf(sbuf, "%s\n",
		    xpc_bool_get_value(obj) ? "true" : "false");
		break;

	case _XPC_TYPE_STRING:
		sbuf_printf(sbuf, "\"%s\"\n",
		    xpc_string_get_string_ptr(obj));
		break;

	case _XPC_TYPE_INT64:
		sbuf_printf(sbuf, "%lld\n",
		    xpc_int64_get_value(obj));
		break;

	case _XPC_TYPE_UINT64:
		sbuf_printf(sbuf, "0x%llx\n",
		    xpc_uint64_get_value(obj));
		break;

	case _XPC_TYPE_DATE:
		sbuf_printf(sbuf, "%llu\n",
		    xpc_date_get_value(obj));
		break;	

	case _XPC_TYPE_UUID:
#ifdef __APPLE__
		memcpy(id, xpc_uuid_get_bytes(obj), sizeof(id));
		uuid_str = (char*) __builtin_alloca(40);
		uuid_unparse(*id, uuid_str);
#else
		id = (struct uuid *)xpc_uuid_get_bytes(obj);
		uuid_to_string(id, &uuid_str, &uuid_status);
#endif
		sbuf_printf(sbuf, "%s\n", uuid_str);
		free(uuid_str);
		break;

	case _XPC_TYPE_ENDPOINT:
		sbuf_printf(sbuf, "<%d>\n", xo->xo_int);
		break;

	case _XPC_TYPE_NULL:
		sbuf_printf(sbuf, "<null>\n");
		break;
	}
}

struct _launch_data {
	uint64_t type;
	union {
		struct {
			union {
				launch_data_t *_array;
				char *string;
				void *opaque;
				int64_t __junk;
			};
			union {
				uint64_t _array_cnt;
				uint64_t string_len;
				uint64_t opaque_size;
			};
		};
		int64_t fd;
		uint64_t  mp;
		uint64_t err;
		int64_t number;
		uint64_t boolean; /* We'd use 'bool' but this struct needs to be used under Rosetta, and sizeof(bool) is different between PowerPC and Intel */
		double float_num;
	};
};

static uint8_t ld_to_xpc_type[] = {
	_XPC_TYPE_INVALID,
	_XPC_TYPE_DICTIONARY,
	_XPC_TYPE_ARRAY,
	_XPC_TYPE_FD,
	_XPC_TYPE_UINT64,
	_XPC_TYPE_DOUBLE,
	_XPC_TYPE_BOOL,
	_XPC_TYPE_STRING,
	_XPC_TYPE_DATA,
	_XPC_TYPE_ERROR,
	_XPC_TYPE_ENDPOINT
};
	
xpc_object_t
ld2xpc(launch_data_t ld)
{
	struct xpc_object *xo;
	xpc_u val;


	if (ld->type > LAUNCH_DATA_MACHPORT)
		return (NULL);
	if (ld->type == LAUNCH_DATA_STRING || ld->type == LAUNCH_DATA_OPAQUE) {
		val.str = malloc(ld->string_len);
		memcpy(__DECONST(void *, val.str), ld->string, ld->string_len);
		xo = _xpc_prim_create(ld_to_xpc_type[ld->type], val, ld->string_len);
	} else if (ld->type == LAUNCH_DATA_BOOL) {
		xo = xpc_bool_create((bool)ld->boolean);
	} else if (ld->type == LAUNCH_DATA_ARRAY) {
		xo = xpc_array_create(NULL, 0);
		for (uint64_t i = 0; i < ld->_array_cnt; i++)
			xpc_array_append_value(xo, ld2xpc(ld->_array[i]));
	} else {
		val.ui = ld->mp;
		xo = _xpc_prim_create(ld_to_xpc_type[ld->type], val, ld->string_len);	
	}
	return (xo);
}

xpc_object_t
xpc_copy_entitlement_for_token(const char *key __unused, audit_token_t *token __unused)
{
	return xpc_bool_create(true);
}

xpc_object_t
xpc_copy_entitlements_for_pid(pid_t pid)
{
	return xpc_bool_create(true);
}


#define XPC_RPORT "XPC remote port"
int
xpc_pipe_routine_reply(xpc_object_t xobj)
{
	struct xpc_object *xo;
	size_t size = 0, msg_size;
	struct xpc_message *message;
	kern_return_t kr;
	int err;

	xo = xobj;
	assert(xo->xo_xpc_type == _XPC_TYPE_DICTIONARY);

	if ((errno = xpc_serialize(xo, NULL, 0, &size)) != 0)
        return errno;

	msg_size = size + sizeof(struct xpc_message);

	if ((message = malloc(msg_size)) == NULL)
		return ENOMEM;

	if ((errno = xpc_serialize(xo, message->data, size, NULL)) != 0)
        return errno;

	message->header.msgh_size = msg_size;
	message->header.msgh_remote_port = xpc_dictionary_copy_mach_send(xobj, XPC_RPORT);
	message->header.msgh_local_port = MACH_PORT_NULL;
	message->size = size;
	kr = mach_msg_send(&message->header);
	if (kr != KERN_SUCCESS)
		err = (kr == KERN_INVALID_TASK) ? EPIPE : EINVAL;
	else
		err = 0;
	free(message);
	return (err);
}

int
xpc_pipe_send(xpc_object_t xobj, mach_port_t dst, mach_port_t local,
    uint64_t id)
{
	struct xpc_object *xo;
	size_t size = 0, msg_size;
	struct xpc_message *message;
	kern_return_t kr;
	int err;

	xo = xobj;
	debugf("obj type is %d", xo->xo_xpc_type);
	if (xo->xo_xpc_type != _XPC_TYPE_DICTIONARY)
		debugf("obj type is %s", _xpc_get_type_name(xobj));
	assert(xo->xo_xpc_type == _XPC_TYPE_DICTIONARY);

	debugf("packing message");
	if ((errno = xpc_serialize(xo, NULL, 0, &size)) != 0)
		return -errno;

	msg_size = size + sizeof(struct xpc_message);
	if ((message = malloc(msg_size)) == NULL)
		return -ENOMEM;

	if ((errno = xpc_serialize(xo, message->data, size, NULL)) != 0)
		return -errno;

	debugf("sending message");
	msg_size = ALIGN(size + sizeof(mach_msg_header_t) + sizeof(size_t) + sizeof(uint64_t));
	message->header.msgh_size = (mach_msg_size_t)msg_size;
	message->header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,
	    MACH_MSG_TYPE_MAKE_SEND);
	message->header.msgh_remote_port = dst;
	message->header.msgh_local_port = local;
	message->id = id;
	message->size = size;
	kr = mach_msg_send(&message->header);
	if (kr != KERN_SUCCESS)
		err = (kr == KERN_INVALID_TASK || kr == MACH_SEND_INVALID_DEST) ? -EPIPE : -EINVAL;
	else
		err = 0;
	free(message);
	return (err);	
}

#define LOG(msg, ...)	\
	do {            \
	debugf("%s:%u: " msg, __FILE__, __LINE__,##__VA_ARGS__);	\
	} while (0)

int
xpc_pipe_receive(mach_port_t local, mach_port_t *remote, xpc_object_t *result,
    uint64_t *id)
{
	struct xpc_recv_message message;
	mach_msg_header_t *request;
	kern_return_t kr;
	mach_msg_trailer_t *tr;
	int data_size;
	struct xpc_object *xo;
	audit_token_t *auditp;

	request = &message.header;
	/* should be size - but what about arbitrary XPC data? */
	request->msgh_size = MAX_RECV;
	request->msgh_local_port = local;
	kr = mach_msg(request, MACH_RCV_MSG |
	    MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) |
	    MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT),
	    0, request->msgh_size, request->msgh_local_port,
	    MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);

	if (kr != KERN_SUCCESS) {
		LOG("mach_msg_receive returned %d\n", kr);
		return (kr == KERN_INVALID_TASK || kr == MACH_RCV_PORT_DIED || kr == MACH_RCV_PORT_CHANGED) ? -EPIPE : -EINVAL;
	}

	*remote = request->msgh_remote_port;
	*id = message.id;
	data_size = (int)message.size;
	LOG("unpacking data_size=%d", data_size);
	if (xpc_deserialize(&xo, message.data, data_size, NULL) != 0)
		return -1;

	tr = (mach_msg_trailer_t *)(((char *)&message) + request->msgh_size);
	auditp = &((mach_msg_audit_trailer_t *)tr)->msgh_audit;

	xo->xo_audit_token = malloc(sizeof(*auditp));
	memcpy(xo->xo_audit_token, auditp, sizeof(*auditp));

	xpc_dictionary_set_mach_send(xo, XPC_RPORT, request->msgh_remote_port);
	xpc_dictionary_set_uint64(xo, XPC_SEQID, message.id);
	xo->xo_flags |= _XPC_FROM_WIRE;

	*result = xo;
	return (0);
}

int
xpc_pipe_try_receive(mach_port_t portset, xpc_object_t *requestobj, mach_port_t *rcvport,
	boolean_t (*demux)(mach_msg_header_t *, mach_msg_header_t *), mach_msg_size_t msgsize __unused,
	int flags __unused)
{
	struct xpc_recv_message message;
	struct xpc_recv_message rsp_message;
	mach_msg_header_t *request;
	kern_return_t kr;
	mach_msg_header_t *response;
	mach_msg_trailer_t *tr;
	int data_size;
	struct xpc_object *xo;
	audit_token_t *auditp;

	request = &message.header;
	response = &rsp_message.header;
	/* should be size - but what about arbitrary XPC data? */
	request->msgh_size = MAX_RECV;
	request->msgh_local_port = portset;
	kr = mach_msg(request, MACH_RCV_MSG |
	    MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) |
	    MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT),
	    0, request->msgh_size, request->msgh_local_port,
	    MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);

	if (kr != 0)
		LOG("mach_msg_receive returned %d\n", kr);
	*rcvport = request->msgh_remote_port;
	if (demux(request, response)) {
		mig_reply_error_t* migError = (mig_reply_error_t*) response;

		if (!(migError->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
			if (migError->RetCode == MIG_NO_REPLY)
				migError->Head.msgh_remote_port = MACH_PORT_NULL;
		}

		if (response->msgh_remote_port != MACH_PORT_NULL)
			(void)mach_msg_send(response);
		/*  can't do anything with the return code
		* just tell the caller this has been handled
		*/
		return (TRUE);
	}
	LOG("demux returned false\n");
	data_size = request->msgh_size;
	LOG("unpacking data_size=%d", data_size);
	if (xpc_deserialize(&xo, message.data, data_size, NULL) != 0)
		return -1;
	/* is padding for alignment enforced in the kernel?*/
	tr = (mach_msg_trailer_t *)(((char *)&message) + request->msgh_size);
	auditp = &((mach_msg_audit_trailer_t *)tr)->msgh_audit;

	xo->xo_audit_token = malloc(sizeof(*auditp));
	memcpy(xo->xo_audit_token, auditp, sizeof(*auditp));

	xpc_dictionary_set_mach_send(xo, XPC_RPORT, request->msgh_remote_port);
	xpc_dictionary_set_uint64(xo, XPC_SEQID, message.id);
	xo->xo_flags |= _XPC_FROM_WIRE;
	*requestobj = xo;
	return (0);
}

int
xpc_call_wakeup(mach_port_t rport, int retcode)
{
	mig_reply_error_t msg;
	int err;
	kern_return_t kr;

	msg.Head.msgh_remote_port = rport;
	msg.RetCode = retcode;
	kr = mach_msg_send(&msg.Head);
	if (kr != KERN_SUCCESS)
		err = (kr == KERN_INVALID_TASK) ? EPIPE : EINVAL;
	else
		err = 0;

	return (err);
}

xpc_object_t
_od_rpc_call(const char *procname, xpc_object_t payload, xpc_pipe_t (*get_pipe)(bool))
{
	printf("STUB _od_rpc_call\n");
	return NULL;
}

int
xpc_pipe_routine(xpc_object_t pipe, void *payload,xpc_object_t *reply)
{
	printf("STUB xpc_pipe_routine\n");
	return 0;
}

void
xpc_dictionary_set_uuid(xpc_object_t xdict, const char *key, const uuid_t uuid)
{
	printf("STUB xpc_dictionary_set_uuid\n");
}

int launch_activate_socket(const char* key, int** fds, size_t* count) {
	// notes for someone implementing this in the future:
	//
	// this function is used in OpenSSH in ssh-agent.c
	//
	// `key` is the socket key in the current process's launchd plist
	// `fds` is a pointer to an array that we allocate that is freed by the caller
	// `count` is the size of that array
	//
	// implementing this requires looking up the current process's launchd plist
	// and reading socket values from there (or talking to launchd to do that,
	// if we can do that; i haven't looked into this much)
	printf("STUB launch_activate_socket\n");
	if (fds)
		*fds = NULL;
	if (count)
		*count = 0;
	return -1;
};

struct os_system_version {
	unsigned int major;
	unsigned int minor;
	unsigned int patch;
};

int os_system_version_get_current_version(struct os_system_version* version_info) {
	char version_string[48] = {0};
	size_t version_string_length = sizeof(version_string);
	char* ptr = NULL;
	int status = 0;

	if ((status = sysctlbyname("kern.osproductversion", version_string, &version_string_length, NULL, 0)) != 0)
		goto out;

	version_info->major = strtoul(version_string, &ptr, 10);
	if (*ptr != '\0') {
		version_info->minor = strtoul(ptr + 1, &ptr, 10);
		if (*ptr != '\0') {
			version_info->patch = strtoul(ptr + 1, &ptr, 10);
		} else {
			version_info->patch = 0;
		}
	} else {
		version_info->minor = 0;
		version_info->patch = 0;
	}

out:
	return status;
};
