/*
 * Copyright (c) 2005-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include <xpc/xpc.h>
#include <xpc/util.h>
#include <xpc/private.h>
//#include "config.h"
#include "launch.h"
#include "launch_priv.h"
//#include "launch_internal.h"
//#include "ktrace.h"

#include <mach/mach.h>
#include <libkern/OSByteOrder.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <sys/syscall.h>
#include <dlfcn.h>

typedef struct _launch *launch_t;

#include "bootstrap.h"
#include "vproc.h"
#include "vproc_priv.h"
//#include "vproc_internal.h"

/* __OSBogusByteSwap__() must not really exist in the symbol namespace
 * in order for the following to generate an error at build time.
 */
extern void __OSBogusByteSwap__(void);

#define host2wire(x)				\
	({ typeof (x) _X, _x = (x);		\
	 switch (sizeof(_x)) {			\
	 case 8:				\
	 	_X = OSSwapHostToLittleInt64(_x);	\
	 	break;				\
	 case 4:				\
	 	_X = OSSwapHostToLittleInt32(_x);	\
	 	break;				\
	 case 2:				\
	 	_X = OSSwapHostToLittleInt16(_x);	\
	 	break;				\
	 case 1:				\
	 	_X = _x;			\
		break;				\
	 default:				\
	 	__OSBogusByteSwap__();		\
		break;				\
	 }					\
	 _X;					\
	 })


#define big2wire(x)				\
	({ typeof (x) _X, _x = (x);		\
	 switch (sizeof(_x)) {			\
	 case 8:				\
	 	_X = OSSwapLittleToHostInt64(_x);	\
	 	break;				\
	 case 4:				\
	 	_X = OSSwapLittleToHostInt32(_x);	\
	 	break;				\
	 case 2:				\
	 	_X = OSSwapLittleToHostInt16(_x);	\
	 	break;				\
	 case 1:				\
	 	_X = _x;			\
		break;				\
	 default:				\
	 	__OSBogusByteSwap__();		\
		break;				\
	 }					\
	 _X;					\
	 })

union _launch_double_u {
	uint64_t iv;
	double dv;
};

#define host2wire_f(x) ({ \
	typeof(x) _F, _f = (x); \
	union _launch_double_u s; \
	s.dv = _f; \
	s.iv = host2wire(s.iv); \
	_F = s.dv; \
	_F; \
})

#define big2wire_f(x) ({ \
	typeof(x) _F, _f = (x); \
	union _launch_double_u s; \
	s.dv = _f; \
	s.iv = big2wire(s.iv); \
	_F = s.dv; \
	_F; \
})


struct launch_msg_header {
	uint64_t magic;
	uint64_t len;
};

#define LAUNCH_MSG_HEADER_MAGIC 0xD2FEA02366B39A41ull

enum {
	LAUNCHD_USE_CHECKIN_FD,
	LAUNCHD_USE_OTHER_FD,
};
struct _launch {
	void	*sendbuf;
	int	*sendfds;
	void	*recvbuf;
	int	*recvfds;
	size_t	sendlen;
	size_t	sendfdcnt;
	size_t	recvlen;
	size_t	recvfdcnt;
	int which;
	int cifd;
	int	fd;
};

static void launch_msg_getmsgs(launch_data_t m, void *context);
static launch_data_t launch_msg_internal(launch_data_t d);
static void launch_mach_checkin_service(launch_data_t obj, const char *key, void *context);

//
//void _xpc_dictionary_set_15663819_hack(launch_data_t dictionary)
//{
//  *(dictionary + 28) |= (char)4;
//}
//

void xpc_launch_routine(int a1, xpc_object_t a2, xpc_object_t *a3)
{
  signed int v4; // eax
  xpc_object_t v5; // r14
  xpc_object_t v6; // rax
  xpc_object_t v7; // [rsp+8h] [rbp-18h]

  v7 = 0LL;
  v4 = xpc_interface_routine(7, a1, a2, &v7, 0, 0);
  if ( v4 )
    goto LABEL_2;
  v5 = v7;
  v6 = xpc_dictionary_get_value(v7, "response");
  if ( !v6 )
  {
    xpc_release(v5);
    v4 = 118;
LABEL_2:
    *a3 = xpc_uint64_create(v4);
    return;
  }
  *a3 = xpc_retain(v6);
  xpc_release(v5);
}

launch_data_t
launch_data_alloc(launch_data_type_t type)
{
    launch_data_t result = NULL;
    char zero = 0;
    
    switch (type){
        case LAUNCH_DATA_DICTIONARY:
            result = (launch_data_t)xpc_dictionary_create(0, 0, 0);
        case LAUNCH_DATA_ARRAY:
            result = (launch_data_t)xpc_array_create(0, 0);
            break;
        case LAUNCH_DATA_INTEGER:
            result = (launch_data_t)xpc_int64_create(0);
            break;
        case LAUNCH_DATA_REAL:
            result = (launch_data_t)xpc_double_create(0);
            break;
        case LAUNCH_DATA_BOOL:
            result = (launch_data_t)xpc_bool_create(0);
            break;
        case LAUNCH_DATA_STRING:
            result = (launch_data_t)xpc_string_create(&zero);
            break;
        case LAUNCH_DATA_OPAQUE:
            result = (launch_data_t)xpc_data_create((const void*)&zero, 1);
            break;
        case LAUNCH_DATA_ERRNO:
            result = (launch_data_t)xpc_uint64_create(0);
            break;
        case LAUNCH_DATA_FD:
        case LAUNCH_DATA_MACHPORT:
            xpc_abort("This is not what you want to do.");
        default:
            break;
    }
    
	return result;
}

// FIXME: this is not properly done. does xpc_get_type work ?
launch_data_type_t
launch_data_get_type(launch_data_t data)
{
    // xpc_type is incopmlete
//    xpc_type_t type = xpc_get_type(data);

#undef bool
#undef errno
//    IS_OBJC_TYPE(data, dictionary)
//                    return LAUNCH_DATA_DICTIONARY;
//    IS_OBJC_TYPE(data, array)
//                    return LAUNCH_DATA_ARRAY;
//    IS_OBJC_TYPE(data, fd)
//                    return LAUNCH_DATA_FD;
//    IS_OBJC_TYPE(data, integer)
//                    return LAUNCH_DATA_INTEGER;
//    IS_OBJC_TYPE(data, double)
//                    return LAUNCH_DATA_REAL;
//    IS_OBJC_TYPE(data, bool)
//                    return LAUNCH_DATA_BOOL;
//    IS_OBJC_TYPE(data, string)
//                    return LAUNCH_DATA_STRING;
//    IS_OBJC_TYPE(data, data)
//                    return LAUNCH_DATA_OPAQUE;
//    IS_OBJC_TYPE(data, errno)
//                    return LAUNCH_DATA_ERRNO;
//    IS_OBJC_TYPE(data, machport)
//                    return LAUNCH_DATA_MACHPORT;
#define bool _Bool
#define errno (*__error())

    return 0;
}

void
launch_data_free(launch_data_t data)
{
    xpc_release(data);
}

size_t
launch_data_dict_get_count(launch_data_t dict)
{
    return xpc_dictionary_get_count(dict);
}

bool
launch_data_dict_insert(launch_data_t dict, launch_data_t what, const char *key)
{
    xpc_dictionary_set_value(dict, what, key);
    xpc_release(what);
    return true;
}

launch_data_t
launch_data_dict_lookup(launch_data_t dict, const char *key)
{
    if (launch_data_get_type(dict) == LAUNCH_DATA_DICTIONARY)
      return (launch_data_t)xpc_dictionary_get_value(dict, key);
    else return 0;
}

bool
launch_data_dict_remove(launch_data_t dict, const char *key)
{
    xpc_dictionary_set_value(dict, key, 0);
    return true;
}

// FIXME: complete...
void
launch_data_dict_iterate(launch_data_t dict, void (*cb)(launch_data_t, const char *, void *), void *context)
{

	if (launch_data_get_type(dict) != LAUNCH_DATA_DICTIONARY)
        return;
    xpc_abort("launch_data_dict_iterate is incomplete.");
    return;
}

bool
launch_data_array_set_index(launch_data_t array, launch_data_t value, size_t ind)
{
    uint64_t count = xpc_array_get_count((xpc_object_t)array);
    
    if (ind == count)
        xpc_array_append_value(array, value);
    else if (ind > count)
        xpc_abort("Out-of-bounds launch array insertion attempt.");
    else
        xpc_array_set_value(array, ind, value);

    xpc_release((xpc_object_t)value);
	return true;
}

launch_data_t
launch_data_array_get_index(launch_data_t array, size_t ind)
{
	return (launch_data_t)xpc_array_get_value(array, ind);
}


launch_data_t
launch_data_array_pop_first(launch_data_t array)
{
//    launch_data_t first = launch_data_array_get_index(array, 0);
//    launch_data_array_set_index(first, NULL, 0);
	return NULL;
}

size_t
launch_data_array_get_count(launch_data_t array)
{
	return xpc_array_get_count(array);
}

bool
launch_data_set_errno(launch_data_t d, int e)
{
	return true;
}

bool
launch_data_set_fd(launch_data_t d, int fd)
{
	xpc_abort("This API doesn't work");
	return true;
}

bool
launch_data_set_machport(launch_data_t d, mach_port_t p)
{
    xpc_abort("This API doesn't work");
    return true;
}

bool
launch_data_set_integer(launch_data_t d, long long n)
{
    xpc_abort("xpc_int64_set_value() is no longer available. Use launch_data_new_integer()");
	return true;
}

bool
launch_data_set_bool(launch_data_t data, bool value)
{
    return xpc_bool_set_value(data, value);
}

bool
launch_data_set_real(launch_data_t data, double value)
{
	return xpc_double_set_value(data, value);
}

bool
launch_data_set_string(launch_data_t data, const char *string)
{
    xpc_string_set_value(data, string);
    return true;
}

bool
launch_data_set_opaque(launch_data_t d, const void *o, size_t os)
{
    xpc_data_set_value(d, o, os);
    return true;
}

int
launch_data_get_errno(launch_data_t d)
{
    return xpc_uint64_get_value(d);
}

int
launch_data_get_fd(launch_data_t d)
{
	return xpc_fd_dup(d);
}

mach_port_t
launch_data_get_machport(launch_data_t d)
{
	return xpc_mach_recv_get_name(d);
}

long long
launch_data_get_integer(launch_data_t d)
{
	return xpc_uint64_get_value(d);
}

bool
launch_data_get_bool(launch_data_t d)
{
	return xpc_bool_get_value(d);
}

double
launch_data_get_real(launch_data_t d)
{
    return xpc_double_get_value(d);
}

const char *
launch_data_get_string(launch_data_t d)
{
	return xpc_string_get_string_ptr(d);
}

void *
launch_data_get_opaque(launch_data_t d)
{
	return xpc_data_get_bytes_ptr(d);
}

size_t
launch_data_get_opaque_size(launch_data_t d)
{
    return xpc_data_get_length(d);
}

int
launchd_getfd(launch_t l)
{
	return -1;
}

launch_t
launchd_fdopen(int fd, int cifd)
{
    return 0;
}

void
launchd_close(launch_t lh, typeof(close) closefunc)
{
    return;
}

size_t
launch_data_pack(launch_data_t d, void *where, size_t len, int *fd_where, size_t *fd_cnt)
{
    return 0;
}

launch_data_t
launch_data_unpack(void *data, size_t data_size, int *fds, size_t fd_cnt, size_t *data_offset, size_t *fdoffset)
{
    return 0;
}

int
launchd_msg_send(launch_t lh, launch_data_t d)
{
    return -1;
}

int
launch_get_fd(void)
{
	return -1;
}

void
launch_msg_getmsgs(launch_data_t m, void *context)
{
}

void
launch_mach_checkin_service(launch_data_t obj, const char *key, void *context __attribute__((unused)))
{
}

// TODO: fixme
launch_data_t
launch_msg(launch_data_t d)
{
    launch_data_t request = xpc_dictionary_create(0, 0, 0);
    launch_data_t response = NULL;

    xpc_dictionary_set_uint64(request, "type", 7);
    xpc_dictionary_set_uint64(request, "handle", 0);
    xpc_dictionary_set_value(request, "request", d);
    if (xpc_get_type(d) == "&OBJC_CLASS___OS_xpc_dictionary" && xpc_dictionary_get_bool(d, "_TargetLocalDomain")){
      xpc_dictionary_set_uint64(request, "type", 6);
    }
    xpc_launch_routine(100, request, &response);
//    if ( xpc_get_type(v3) == &OBJC_CLASS___OS_xpc_dictionary )
//      xpc_dictionary_set_15663819_hack((int64_t)v3);
    xpc_release(request);
    return (launch_data_t)response;
}

extern kern_return_t vproc_mig_set_security_session(mach_port_t, uuid_t, mach_port_t);

static inline bool
uuid_data_is_null(launch_data_t d)
{
	bool result = false;
	if (launch_data_get_type(d) == LAUNCH_DATA_OPAQUE && launch_data_get_opaque_size(d) == sizeof(uuid_t)) {
		uuid_t existing_uuid;
		memcpy(existing_uuid, launch_data_get_opaque(d), sizeof(uuid_t));

		/* A NULL UUID tells us to keep the session inherited from the parent. */
		result = (bool)uuid_is_null(existing_uuid);
	}

	return result;
}

int
launchd_msg_recv(launch_t lh, void (*cb)(launch_data_t, void *), void *context)
{
    return -1;
}

launch_data_t
launch_data_copy(launch_data_t data)
{
    return xpc_copy(data);
}

launch_data_t
launch_data_new_errno(int e)
{
	return xpc_uint64_create(e);
}

launch_data_t
launch_data_new_fd(int fd)
{
    return xpc_fd_create(fd);
}

launch_data_t
launch_data_new_machport(mach_port_t p)
{
    return xpc_mach_recv_create(p);
}

launch_data_t
launch_data_new_integer(long long n)
{
	return xpc_uint64_create(n);
}

launch_data_t
launch_data_new_bool(bool b)
{
    return xpc_bool_create(b);
}

launch_data_t
launch_data_new_real(double d)
{
	return xpc_double_create(d);
}

launch_data_t
launch_data_new_string(const char *s)
{
	return xpc_string_create(s);
}

launch_data_t
launch_data_new_opaque(const void *data, size_t size)
{
	return xpc_data_create(data,size);
}

void
load_launchd_jobs_at_loginwindow_prompt(int flags __attribute__((unused)), ...)
{
    xpc_object_t response = NULL;
    kern_return_t ret;
    mach_port_t port;
    
    xpc_globals_t globals = _xpc_globals();
    xpc_object_t request = xpc_dictionary_create(0, 0, 0);
    
    xpc_dictionary_set_uint64(request, "type", 7);
    xpc_dictionary_set_uint64(request, "handle", 0);
    xpc_dictionary_set_uint64(request, "uid", 0xFFFFFFFF);
    
    ret = xpc_domain_routine(823, request, &response);
    if (ret){
        syslog(LOG_ERR, "Could not load launchd jobs at loginwindow prompt: %d: %s", ret, xpc_strerror(ret));
    } else {
        port = xpc_dictionary_extract_mach_send(response, "port");
        if (port + 1 < 2)
            syslog(LOG_ERR, "Bad response from launchd.");
        else {
            ret = task_set_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, port);
            if (ret){
//                os_assert_log(ret);
                xpc_abort("failed to get bootstrap port");
            }
            ret = xpc_mach_port_release(bootstrap_port);
//            if (ret) os_assumes_log(ret, 4LL);
            bootstrap_port = port;
            ret = xpc_mach_port_retain_send(port);
//            if (ret) os_assumes_log(ret, 4LL);
            ret = xpc_mach_port_release(globals->bootstrap_port);
//            if (ret) os_assumes_log(ret, 4LL);
            globals->bootstrap_port = port;
            ret = xpc_mach_port_retain_send(port);
//            if (ret) os_assumes_log(ret, 4LL);
            xpc_release(globals->bootstrap_pipe);
            globals->bootstrap_pipe = xpc_pipe_create_from_port(port, 4);
        }
        xpc_release(response);
    }
    xpc_release(request);
}

pid_t
create_and_switch_to_per_session_launchd(const char *login __attribute__((unused)), int flags, ...)
{
    kern_return_t ret;
    xpc_object_t response;
    xpc_object_t request = xpc_dictionary_create(0, 0, 0);
    
    xpc_dictionary_set_uint64(request, "type", 7);
    xpc_dictionary_set_uint64(request, "handle", 0);
    xpc_dictionary_set_uint64(request, "flags", flags);
    
    ret = xpc_domain_routine(823, request, &response);
    if (ret){
        syslog(LOG_ERR, "Could not switch to per-user context: %d: %s", ret, xpc_strerror(ret));
        ret = -1;
    } else {
        xpc_release(response);
        ret = 1;
    }
    xpc_release(request);
    return ret;
    
}
