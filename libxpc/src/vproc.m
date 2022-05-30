/*
 * Copyright (c) 1999-2012 Apple Inc. All rights reserved.
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
#include "vproc.h"
#include "vproc_priv.h"
//#include "vproc_internal.h"

#include <dispatch/dispatch.h>
#include <libproc.h>
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <libkern/OSAtomic.h>
#include <sys/syscall.h>
//#include <sys/event.h>
#include <System/sys/fileport.h>
//#include <os/assumes.h>

//#if HAVE_QUARANTINE
//#include <quarantine.h>
//#endif

//#include "launch.h"
#include "launch_priv.h"
//#include "launch_internal.h"
//#include "ktrace.h"
//
//#include "job.h"
//
//#include "helper.h"
//#include "helperServer.h"
//
//#include "reboot2.h"

#define likely(x) __builtin_expect((bool)(x), true)
#define unlikely(x) __builtin_expect((bool)(x), false)

#define _vproc_set_crash_log_message(x)
#define _audit_session_self(v) (mach_port_t)syscall(SYS_audit_session_self)
#define _audit_session_join(s) (au_asid_t)syscall(SYS_audit_session_join, session)

void _vproc_transactions_enable_internal(void *arg);
void _vproc_transaction_begin_internal(void *arg __unused);
void _vproc_transaction_end_internal(void *arg __unused);

#pragma mark vproc Object
struct vproc_s {
	int32_t refcount;
	mach_port_t j_port;
};

vproc_t
vprocmgr_lookup_vproc(const char *label)
{
	return 0;
}

vproc_t
vproc_retain(vproc_t vp)
{
	return &_xpc_bool_false;
}

void
vproc_release(vproc_t vp)
{
	return;
}

#pragma mark Transactions

void
_vproc_transactions_enable(void)
{
    xpc_transaction_enable();
}

void
_vproc_transaction_begin(void)
{
    xpc_transaction_begin();
}

vproc_transaction_t
vproc_transaction_begin(vproc_t virtual_proc)
{
    _vproc_transaction_begin();
    return (vproc_transaction_t)&_vproc_transaction_begin;
}

void
_vproc_transaction_end(void)
{
    xpc_transaction_end();
}

void
vproc_transaction_end(vproc_t vp __unused, vproc_transaction_t vpt __unused)
{
	_vproc_transaction_end();
}

size_t
_vproc_transaction_count(void)
{
    return 0;
}

size_t
_vproc_standby_count(void)
{
	return 0;
} 

size_t
_vproc_standby_timeout(void)
{
	return 0;
}

bool
_vproc_pid_is_managed(pid_t p)
{
	return false;
}

kern_return_t
_vproc_transaction_count_for_pid(pid_t p, int32_t *count, bool *condemned)
{
    return 0;
}

void
_vproc_transaction_try_exit(int status)
{
	xpc_globals_t globals = _xpc_globals();
	if (globals->transaction_count == 0)
		_exit(status);
}

void
_vproc_standby_begin(void)
{
}

vproc_standby_t
vproc_standby_begin(vproc_t vp __unused)
{
	return (vproc_standby_t)vproc_standby_begin;
}

void
_vproc_standby_end(void)
{
}

void
vproc_standby_end(vproc_t vp __unused, vproc_standby_t vpt __unused)
{
}

void
_vproc_transaction_set_clean_callback(dispatch_queue_t targetq, void *ctx, dispatch_function_t func)
{
}


#pragma mark Miscellaneous SPI
kern_return_t
_vproc_grab_subset(mach_port_t bp, mach_port_t *reqport, mach_port_t *rcvright,
	struct launch_data *outval, mach_port_array_t *ports,
	mach_msg_type_number_t *portCnt)
{
    return KERN_NOT_SUPPORTED;
}

vproc_err_t
_vprocmgr_move_subset_to_user(uid_t target_user, const char *session_type, uint64_t flags)
{
    return &_xpc_bool_false;
}

vproc_err_t
_vprocmgr_switch_to_session(const char *target_session, unsigned flags __attribute__((unused)))
{
    int ret;
    xpc_object_t request = xpc_dictionary_create(0, 0, 0);
    xpc_object_t response = NULL;
    mach_port_t new_bsport = MACH_PORT_NULL;
    
    xpc_dictionary_set_uint64(request, "type", 7);
    xpc_dictionary_set_mach_send(request, "domain-port", bootstrap_port);
    xpc_dictionary_set_uint64(request, "handle", 0);
    xpc_dictionary_set_string(request, "session", target_session);
    
    ret = xpc_domain_routine(824, request, &response);
    if (!ret){
        new_bsport = xpc_dictionary_extract_mach_send(request, "exc-port");
        if ( new_bsport + 1 >= 2 ){
            ret = xpc_set_exception_port(0, new_bsport);
//            if (ret) os_assumes_log(ret, new_bsport);
//            status = xpc_mach_port_release(new_bsport);
            ret = mach_port_deallocate(mach_task_self(), new_bsport);
//            if (ret) os_assumes_log(ret, new_bsport);
        }
        xpc_release(response);
    } else {
        syslog(LOG_ERR, "Could not adopt Background session: %d: %s", ret, xpc_strerror(ret));
        ret = &_xpc_bool_false;
    }
    xpc_release(request);
    return ret;
}

vproc_err_t 
_vprocmgr_detach_from_console(vproc_flags_t flags __attribute__((unused)))
{
	return _vprocmgr_switch_to_session(VPROCMGR_SESSION_BACKGROUND, 0);
}

vproc_err_t
_vproc_post_fork_ping(void)
{
	return _vprocmgr_switch_to_session(VPROCMGR_SESSION_BACKGROUND, 0);
}

vproc_err_t
_vprocmgr_init(const char *session_type)
{
	return (vproc_err_t)&_xpc_bool_false;
}

struct xpc_spawnattr_packed {
    uint32_t packed_size;   // 0
    uint32_t _data_4;       // 4
    uint64_t _data_8;       // 8
    uint32_t argc;          // 16
    uint32_t argv_index;    // 20
    uint16_t envc;         // 24
    uint8_t  _data_26;      // 26
    uint8_t  _data_27;      // 27
    uint16_t env_index;      // 28
    uint16_t umask;         // 30
    uint32_t binpref_cnt;      // 32
    uint16_t binpref_index;      // 36
    uint16_t _data_38;      // 38
    uint64_t _data_40;      // 40
    uint64_t _data_48;      // 48
    uint64_t _data_56;      // 56
    uint64_t _data_64;      // 64
    uint32_t _data_72;      // 72
    uint32_t chdir_index;      // 76
    uint64_t _data_80;      // 80
    uint64_t _data_88;      // 88
    uint32_t _data_96;      // 96
    bool wait_for_debugger; // 100
    uint8_t  _data_101;     // 101
    uint16_t _data_102;     // 102
    uint64_t _data_104;     // 104
    uint64_t _data_112;     // 112
    uint64_t _data_120;     // 120
    uint64_t _data_128;     // 128
    uint64_t _data_136;     // 136
    uint64_t _data_144;     // 144
    uint64_t _data_152;     // 152
    uint64_t _data_160;     // 160
    uint32_t _data_168;     // 168
    uint8_t  _data_172;     // 172
    uint8_t  _data_173;     // 173
    uint16_t binprefs;     // 174
    char     _data_175;     // 175
    char     _strings[];    // 176
} __attribute__((packed)); // apple packs this
typedef struct xpc_spawnattr_packed *xpc_spawnattr_packed_t ;

size_t xpc_spawnattr_pack_string(struct xpc_spawnattr_packed *attrs, size_t *str_index,
                                 size_t *strlen_left, const char *string)
{
    size_t result;
    strcpy(((char *)attrs->_strings) + *str_index, string);
    result = strlen(string) + 1;
    *str_index += result;
    *strlen_left -= result;
    return result;
}

// Spent quite some time decompiling this
// TODO: rewrite this function's mumbojumbo code flow.
pid_t
_spawn_via_launchd(const char *label, const char *const *argv, const struct spawn_via_launchd_attr *spawnattr, int struct_version)
{
    xpc_spawnattr_packed_t packedattrs;
    size_t expected_length;
    const char **ptr = NULL;
    const char *string;
    signed int _data_104;
    xpc_object_t request;
    kern_return_t ret;
    xpc_object_t response = NULL;
    size_t strleft;
    size_t strindex = 0;
    bool v8;
    
    if (struct_version < 3 )
        xpc_abort("struct versions less than 3 are no longer supported.");
LABEL_13:
    
    v8 = 0;
    if (spawnattr != NULL){
        expected_length = strlen(spawnattr->spawn_path) + sizeof(struct xpc_spawnattr_packed);
        
        if(!argv){
            if (spawnattr->spawn_env){
                ptr = (const char**)spawnattr->spawn_env;
                while (ptr++){
                    expected_length += strlen(*ptr) + 1;
                }
                
                if (spawnattr->spawn_chdir)
                    expected_length += strlen(spawnattr->spawn_chdir) + 1;
            
                if (spawnattr->spawn_binpref_cnt)
                    expected_length += 4 * spawnattr->spawn_binpref_cnt;
                goto LABEL_21;
            }
        }
        
        string = *argv;
        v8 = 1;
        
    } else {
        string = *argv;
        expected_length = strlen(*argv) + 177;
        v8 = argv != 0LL;
    }
    
    
    if (string) {
        while (string++) expected_length += strlen(string) + 1;
    }
    
    if (spawnattr)
      goto LABEL_13;
    
LABEL_21:
    packedattrs = calloc(1, expected_length);
    if (packedattrs == NULL){
        errno = ENOMEM;
        return (uint32_t)-1;
    }
    
    packedattrs->packed_size = expected_length;
    strleft = expected_length - sizeof(struct xpc_spawnattr_packed) - 1;
    
    if (!spawnattr || spawnattr->spawn_path == NULL)
        xpc_spawnattr_pack_string(packedattrs, &strindex, &strleft, spawnattr->spawn_path);
    
    if (argv != NULL){
        packedattrs->argv_index = strindex;
        ptr = (const char**)argv;
            while (ptr++){
                xpc_spawnattr_pack_string(packedattrs, &strindex, &strleft, *ptr);
                ++packedattrs->argc;
            }
    }
    _data_104 = 256;
    if (spawnattr){
        if (spawnattr->spawn_env){
            packedattrs->env_index = strindex;
            if (*spawnattr->spawn_env){
                ptr = (const char**)spawnattr->spawn_env;
                while (ptr++) {
                    xpc_spawnattr_pack_string(packedattrs, &strindex, &strleft, *ptr);
                    ++packedattrs->envc;
                }
            }
        }
        
        if (spawnattr->spawn_chdir){
            packedattrs->chdir_index = strindex;
            xpc_spawnattr_pack_string(packedattrs, &strindex, &strleft, spawnattr->spawn_chdir);
        }
        
        
        if (spawnattr->spawn_binpref_cnt){
            packedattrs->binpref_index = strindex;
            packedattrs->binpref_cnt = spawnattr->spawn_binpref_cnt;
            size_t cnt = 4 * spawnattr->spawn_binpref_cnt;
            memcpy(((char *)&packedattrs->binprefs) + strindex, spawnattr->spawn_binpref, cnt);
            strindex += cnt;
            strleft -= cnt;
        }
        
        if (spawnattr->spawn_flags & SPAWN_VIA_LAUNCHD_STOPPED)
            packedattrs->wait_for_debugger = true; // TODO: check data type
        
        _data_104 = 512;
        if ((spawnattr->spawn_flags & SPAWN_VIA_LAUNCHD_TALAPP) == 0){
            _data_104 = 256;
            if (spawnattr->spawn_flags & 4)
                goto LABEL_46;
        }
    }
    
    packedattrs->_data_104 = _data_104;
    
LABEL_46:
    
    
    if (spawnattr){
        if(spawnattr->spawn_umask)
            packedattrs->umask = *spawnattr->spawn_umask;
        if (struct_version >= 4){
            if (struct_version == 4)
                packedattrs->_data_27 = spawnattr->spawn_offset_88;
            
            // struct_version >= 5
            
            if (*spawnattr->spawn_env){
                packedattrs->_data_173 |= 0x10;
                packedattrs->_data_152 = *spawnattr->spawn_env;
            }
        }
    }
    
    request = xpc_dictionary_create(0, 0, 0);
    xpc_dictionary_set_uint64(request, "type", 7);
    xpc_dictionary_set_uint64(request, "handle", 0);
    xpc_dictionary_set_string(request, "label", label);
    xpc_dictionary_set_data(request, "attr", packedattrs, expected_length);
    
    ret = xpc_domain_routine(817, request, &response);
    if (ret){
        errno = ret;
        return -1;
    }
    else {
        if (spawnattr)
            *spawnattr->spawn_observer_port = xpc_dictionary_extract_mach_recv(response, "observerport");
        ret = xpc_dictionary_get_int64(response, "pid");
        xpc_release(response);
    }
    xpc_release(request);
    free(packedattrs);
    return ret;
}

kern_return_t
mpm_wait(mach_port_t ajob __attribute__((unused)), int *wstatus)
{
	*wstatus = 0;
	return 0;
}

kern_return_t
mpm_uncork_fork(mach_port_t ajob __attribute__((unused)))
{
	return KERN_FAILURE;
}

kern_return_t
_vprocmgr_getsocket(name_t sockpath)
{
	return 0;
}

vproc_err_t
_vproc_get_last_exit_status(int *wstatus)
{
	int64_t val;

	if (vproc_swap_integer(NULL, VPROC_GSK_LAST_EXIT_STATUS, 0, &val) == 0) {
		*wstatus = (int)val;
		return NULL;
	}

	return (vproc_err_t)&_xpc_bool_false;
}

vproc_err_t
_vproc_send_signal_by_label(const char *label, int sig)
{
    xpc_object_t request = xpc_dictionary_create(0, 0, 0);
    xpc_object_t response = NULL;
    uint64_t ret;
    
    xpc_dictionary_set_uint64(request, "type", 7);
    xpc_dictionary_set_uint64(request, "handle", 0);
    xpc_dictionary_set_mach_send(request, "domain-port", bootstrap_port);
    xpc_dictionary_set_int64(request, "signal", sig);
    xpc_dictionary_set_string(request, "name", label);
    
    ret = xpc_domain_routine(812, request, &response);
    if (ret){
        syslog(LOG_ERR, "Could not signal service: %llu: %s", ret, xpc_strerror(ret));
        ret = &_xpc_bool_false;
    } else {
        xpc_release(response);
        ret = 0;
    }
    xpc_release(request);
    return (vproc_err_t)ret;
}

vproc_err_t
_vprocmgr_log_forward(mach_port_t mp, void *data, size_t len)
{
	return (vproc_err_t)&_xpc_bool_false;
}

XPC_NORETURN
vproc_err_t
_vprocmgr_log_drain(vproc_t vp __attribute__((unused)), pthread_mutex_t *mutex, _vprocmgr_log_drain_callback_t func)
{
    // why???
    while (1) sleep(~0);
}

xpc_object_t vproc_create_request()
{
    xpc_object_t request = xpc_dictionary_create(0, 0, 0);
    xpc_dictionary_set_uint64(request, "type", 7);
    xpc_dictionary_set_mach_send(request, "domain-port", bootstrap_port);
    xpc_dictionary_set_uint64(request, "handle", 0);
    xpc_dictionary_set_bool(request, "self", 1);
    return request;
}

vproc_err_t
xpc_vproc_routine(int routine, xpc_object_t msg, xpc_object_t* out)
{
  return xpc_interface_routine(6, routine, msg, out, 0, 0);
}


vproc_err_t
vproc_swap_integer(vproc_t vp, vproc_gsk_t key, int64_t *inval, int64_t *outval)
{
    xpc_globals_t globals = _xpc_globals();
    uint64_t ret = 0;
    
    switch (key) {
        case VPROC_GSK_GLOBAL_ON_DEMAND:
            if (inval != NULL)
              return _vproc_set_global_on_demand(*inval != 0);
            ret = &_xpc_bool_false;
            break;
        case VPROC_GSK_IS_MANAGED:
            *outval = globals->xpc_is_managed; // set this to xpc_is_managed
            break;
        default: {
            xpc_object_t request = vproc_create_request();
            xpc_object_t response = NULL;
            if (inval){
                xpc_dictionary_set_uint64(request, "ingsk", key);
                xpc_dictionary_set_bool(request, "set", 1);
                xpc_dictionary_set_int64(request, "in", *inval);
            }
            if (outval){
                xpc_dictionary_set_uint64(request, "outgsk", key);
                xpc_dictionary_set_bool(request, "get", 1);
            }
            if ((unsigned int)xpc_vproc_routine(301, request, &response)){
                ret = (uint64_t)vproc_swap_integer;
                xpc_release(response);
            }
            else if (outval)
                *outval = xpc_dictionary_get_int64(response, "out");
            xpc_release(request);
            break;
        }
    }

	return (vproc_err_t)ret;
}

vproc_err_t
vproc_swap_complex(vproc_t vp, vproc_gsk_t key, launch_data_t inval, launch_data_t *outval)
{
    xpc_object_t request = vproc_create_request();
    xpc_object_t response = NULL;
    launch_data_t out_ldata = NULL;
    launch_data_type_t ldata_type;
    uint64_t ret = 0;
    
    if (inval){
        xpc_dictionary_set_uint64(request, "ingsk", key);
        xpc_dictionary_set_bool(request, "set", 1);
        xpc_dictionary_set_value(request, "in", inval);
    }
    if (outval){
        xpc_dictionary_set_uint64(request, "outgsk", key);
        xpc_dictionary_set_bool(request, "get", 1);
    }
    
    ret = xpc_vproc_routine(301, request, &response);
    if (!ret){
        out_ldata = (launch_data_t)xpc_dictionary_get_value(response, "out");
        if (out_ldata != NULL){
            ldata_type = launch_data_get_type(out_ldata);
            if (ldata_type == LAUNCH_DATA_DICTIONARY || ldata_type == LAUNCH_DATA_STRING)
                *outval = xpc_retain(out_ldata);
        }
        xpc_release(response);
    } else { // ret != 0
        ret = vproc_swap_complex;
        if (ret == ENOTSUP)
            syslog(LOG_ERR, "Swap operation not supported: %llu", ret);
//        if (ret != 135) os_assumes_log(ret, request);
    }
    
    xpc_release(request);
    return (vproc_err_t)ret;
}

vproc_err_t
vproc_swap_string(vproc_t vp, vproc_gsk_t key, const char *instr, char **outstr)
{
	launch_data_t instr_data = instr ? launch_data_new_string(instr) : NULL;
	launch_data_t outstr_data = NULL;

	vproc_err_t verr = vproc_swap_complex(vp, key, instr_data, &outstr_data);
	if (!verr && outstr) {
		if (launch_data_get_type(outstr_data) == LAUNCH_DATA_STRING) {
			*outstr = strdup(launch_data_get_string(outstr_data));
		} else {
			verr = (vproc_err_t)vproc_swap_string;
		}
		launch_data_free(outstr_data);
	}
	if (instr_data) {
		launch_data_free(instr_data);
	}

	return verr;
}

vproc_err_t
_vproc_kickstart_by_label(const char *label __unused, pid_t *out_pid __unused,
                          mach_port_t *out_port_name __unused, mach_port_t *out_obsrvr_port __unused,
                          vproc_flags_t flags __unused)
{
	return (vproc_err_t)&_xpc_bool_false;
}

vproc_err_t
_vproc_set_global_on_demand(bool state)
{
    xpc_object_t request = xpc_dictionary_create(0, 0, 0);
    xpc_object_t response = NULL;
    uint64_t ret = 0;
    
    xpc_dictionary_set_uint64(request, "type", 7);
    xpc_dictionary_set_uint64(request, "handle", 0);
    if (xpc_domain_routine(state ^ 0x327, request, &response) != 0)
        ret = &_xpc_bool_false;
    else
        xpc_release(response);
    
    xpc_release(request);
    return ret;
}

void
_vproc_logv(int pri, int err, const char *msg, va_list ap)
{
}

void
_vproc_log(int pri, const char *msg, ...)
{
}

void
_vproc_log_error(int pri, const char *msg, ...)
{
}

/* The type naming convention is as follows:
 * For requests...
 *     union __RequestUnion__<userprefix><subsystem>_subsystem
 * For replies...
 *     union __ReplyUnion__<userprefix><subsystem>_subsystem
 */
//union maxmsgsz {
//	union __RequestUnion__helper_downcall_launchd_helper_subsystem req;
//	union __ReplyUnion__helper_downcall_launchd_helper_subsystem rep;
//};

//const size_t vprocmgr_helper_maxmsgsz = sizeof(union maxmsgsz);

kern_return_t
helper_recv_wait(mach_port_t p, int status)
{
#if __LAUNCH_MACH_PORT_CONTEXT_T_DEFINED__
	mach_port_context_t ctx = status;
#else
	mach_vm_address_t ctx = status;
#endif

	return (errno = mach_port_set_context(mach_task_self(), p, ctx));
}

int
launch_wait(mach_port_t port)
{
    int ret;
    xpc_object_t response = NULL;

    ret = xpc_pipe_receive(port, response, 0);
    if (ret){
      errno = ret;
      ret = -1;
    } else {
      ret = xpc_dictionary_get_int64(response, "status");
      xpc_release(response);
    }
    return ret;
}

launch_data_t
launch_socket_service_check_in(void)
{
//	launch_data_t reply = NULL;
//
//	size_t big_enough = 10 * 1024;
//	void *buff = malloc(big_enough);
//	if (buff) {
//		launch_data_t req = launch_data_new_string(LAUNCH_KEY_CHECKIN);
//		if (req) {
//			size_t sz = launch_data_pack(req, buff, big_enough, NULL, NULL);
//			if (sz) {
//				vm_address_t sreply = 0;
//				mach_msg_size_t sreplyCnt = 0;
//				mach_port_array_t fdps = NULL;
//				mach_msg_size_t fdpsCnt = 0;
//				kern_return_t kr = vproc_mig_legacy_ipc_request(bootstrap_port, (vm_address_t)buff, sz, NULL, 0, &sreply, &sreplyCnt, &fdps, &fdpsCnt, _audit_session_self());
//				if (kr == BOOTSTRAP_SUCCESS) {
//					int fds[128];
//
//					size_t i = 0;
//					size_t nfds = fdpsCnt / sizeof(fdps[0]);
//					for (i = 0; i < nfds; i++) {
//						fds[i] = fileport_makefd(fdps[i]);
//						(void)mach_port_deallocate(mach_task_self(), fdps[i]);
//					}
//
//					size_t dataoff = 0;
//					size_t fdoff = 0;
//					reply = launch_data_unpack((void *)sreply, sreplyCnt, fds, nfds, &dataoff, &fdoff);
//					reply = launch_data_copy(reply);
//
//					mig_deallocate(sreply, sreplyCnt);
//					mig_deallocate((vm_address_t)fdps, fdpsCnt);
//				}
//			}
//
//			launch_data_free(req);
//		}
//
//		free(buff);
//	}
//
//	return reply;
    
    xpc_object_t response = NULL;
    xpc_object_t request = xpc_dictionary_create(0, 0, 0);
    
    xpc_dictionary_set_uint64(request, "type", 6LL);
    xpc_dictionary_set_uint64(request, "handle", 0LL);
    xpc_launch_routine(101, request, &response);
    xpc_release(request);
    return 0;
}
