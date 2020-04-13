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
#include <mach/mach.h>
#include <xpc/launchd.h>
#include "xpc_internal.h"
#include <assert.h>
#include <math.h>

xpc_object_t
xpc_dictionary_create(const char * const *keys, const xpc_object_t *values,
    size_t count)
{
	struct xpc_object *xo;
	size_t i;
	xpc_u val = {0};

	xo = _xpc_prim_create(_XPC_TYPE_DICTIONARY, val, count);
	
	for (i = 0; i < count; i++)
		xpc_dictionary_set_value(xo, keys[i], values[i]);
	
	return (xo);
}

xpc_object_t
xpc_dictionary_create_reply(xpc_object_t original)
{
	struct xpc_object *xo, *xo_orig;
	xpc_u val;

	if (xpc_get_type(original) != XPC_TYPE_DICTIONARY)
		return NULL;

	xo_orig = original;
	if ((xo_orig->xo_flags & _XPC_FROM_WIRE) == 0)
	{
		debugf("xpc_dictionary_create_reply: not from wire!");
		return (NULL);
	}
	xo_orig->xo_flags &= ~_XPC_FROM_WIRE;

	// TODO: Apple puts a reference to original into retval
	return xpc_dictionary_create(NULL, NULL, 0);
}

void
xpc_dictionary_get_audit_token(xpc_object_t xdict, audit_token_t *token)
{
	struct xpc_object *xo;

	xo = xdict;
	if (xo->xo_audit_token != NULL)
		memcpy(token, xo->xo_audit_token, sizeof(*token));
}
void
xpc_dictionary_set_mach_recv(xpc_object_t xdict, const char *key, mach_port_t port)
{
	struct xpc_object *xo = xdict;
	struct xpc_object *xotmp;
	xpc_u val;

	val.port = port;
	xotmp = _xpc_prim_create(_XPC_TYPE_ENDPOINT, val, 0);

	xpc_dictionary_set_value(xdict, key, xotmp);
}

void
xpc_dictionary_set_mach_send(xpc_object_t xdict, const char *key, mach_port_t port)
{
	struct xpc_object *xotmp;
	xpc_u val;

	val.port = port;
	xotmp = _xpc_prim_create(_XPC_TYPE_ENDPOINT, val, 0);

	xpc_dictionary_set_value(xdict, key, xotmp);
}

mach_port_t
xpc_dictionary_copy_mach_send(xpc_object_t xdict, const char *key)
{
	struct xpc_object *xo;
	const struct xpc_object *xotmp;

#if 0
	xo = xdict;
	if (nvlist_exists_type(xo->xo_nv, key, NV_TYPE_ENDPOINT))
		return (nvlist_get_number(xo->xo_nv, key));
	else if (nvlist_exists_binary(xo->xo_nv, key)) {
		xotmp = (void *)nvlist_get_number(xo->xo_nv, key);
		return (xotmp->xo_uint);
	}
	return (0);
#endif
}

void
xpc_dictionary_set_value(xpc_object_t xdict, const char *key, xpc_object_t value)
{
	struct xpc_object *xo, *xotmp;
	struct xpc_dict_head *head;
	struct xpc_dict_pair *pair;

	xo = xdict;
	head = &xo->xo_dict;

	TAILQ_FOREACH(pair, head, xo_link) {
		if (!strcmp(pair->key, key)) {
			pair->value = value;
			return;
		}
	}

	xo->xo_size++;
	pair = malloc(sizeof(struct xpc_dict_pair));
	size_t len = strlen(key);
	char* str = malloc(len + 1);
	strncpy(str, key, len);
	str[len] = '\0';
	pair->key = str;
	pair->value = value;
	TAILQ_INSERT_TAIL(&xo->xo_dict, pair, xo_link);
	xpc_retain(value);
}

xpc_object_t
xpc_dictionary_get_value(xpc_object_t xdict, const char *key)
{
	struct xpc_object *xo;
	struct xpc_dict_head *head;
	struct xpc_dict_pair *pair;

	xo = xdict;
	head = &xo->xo_dict;

	TAILQ_FOREACH(pair, head, xo_link) {
		if (!strcmp(pair->key, key))
			return (pair->value);
	}

	return (NULL);
}

size_t
xpc_dictionary_get_count(xpc_object_t xdict)
{
	struct xpc_object *xo;

	xo = xdict;
	return (xo->xo_size);
}

void
xpc_dictionary_set_bool(xpc_object_t xdict, const char *key, bool value)
{;
	struct xpc_object *xo, *xotmp;

	xo = xdict;
	xotmp = xpc_bool_create(value);
	xpc_dictionary_set_value(xdict, key, xotmp);
}

void
xpc_dictionary_set_int64(xpc_object_t xdict, const char *key, int64_t value)
{
	struct xpc_object *xo, *xotmp;

	xo = xdict;
	xotmp = xpc_int64_create(value);
	xpc_dictionary_set_value(xdict, key, xotmp);
}

void
xpc_dictionary_set_uint64(xpc_object_t xdict, const char *key, uint64_t value)
{
	struct xpc_object *xo, *xotmp;

	xo = xdict;
	xotmp = xpc_uint64_create(value);
	xpc_dictionary_set_value(xdict, key, xotmp); 
}

void
xpc_dictionary_set_string(xpc_object_t xdict, const char *key, const char *value)
{
	struct xpc_object *xo, *xotmp;

	xo = xdict;
	xotmp = xpc_string_create(value);
	xpc_dictionary_set_value(xdict, key, xotmp);
}

bool xpc_dictionary_get_bool(xpc_object_t xdict, const char* key) {
	xpc_object_t xo = xpc_dictionary_get_value(xdict, key);
	if (xo)
		return xpc_bool_get_value(xo);
	return false;
};

int64_t xpc_dictionary_get_int64(xpc_object_t xdict, const char* key) {
	xpc_object_t xo = xpc_dictionary_get_value(xdict, key);
	if (xo)
		return xpc_int64_get_value(xo);
	return 0;
};

uint64_t xpc_dictionary_get_uint64(xpc_object_t xdict, const char* key) {
	xpc_object_t xo = xpc_dictionary_get_value(xdict, key);
	if (xo)
		return xpc_uint64_get_value(xo);
	return 0;
};

const char* xpc_dictionary_get_string(xpc_object_t xdict, const char* key) {
	xpc_object_t xo = xpc_dictionary_get_value(xdict, key);
	if (xo)
		return xpc_string_get_string_ptr(xo);
	return NULL;
}

double xpc_dictionary_get_double(xpc_object_t xdict, const char* key) {
	xpc_object_t xo = xpc_dictionary_get_value(xdict, key);
	if (xo)
		return xpc_double_get_value(xo);
	return NAN;
};

bool
xpc_dictionary_apply(xpc_object_t xdict, xpc_dictionary_applier_t applier)
{
	struct xpc_object *xo, *xotmp;
	struct xpc_dict_head *head;
	struct xpc_dict_pair *pair;

	xo = xdict;
	head = &xo->xo_dict;

	TAILQ_FOREACH(pair, head, xo_link) {
		if (!applier(pair->key, pair->value))
			return (false);
	}

	return (true);
}

const void *
xpc_dictionary_get_data(xpc_object_t xdict, const char *key, size_t *length)
{

}

void
xpc_dictionary_set_data(xpc_object_t xdict, const char *key, const void *bytes, size_t length)
{

}

xpc_connection_t
xpc_dictionary_get_remote_connection(xpc_object_t xdict)
{
	return NULL;
}

int
xpc_dictionary_dup_fd(xpc_object_t xdict, const char *key)
{
	return -1;
}

void
xpc_dictionary_set_fd(xpc_object_t xdict, const char *key, int fd)
{
}

void
xpc_dictionary_set_double(xpc_object_t xdict, const char *key, double value) {
}

const uint8_t *
xpc_dictionary_get_uuid(xpc_object_t xdict, const char *key)
{
	return NULL;
}
