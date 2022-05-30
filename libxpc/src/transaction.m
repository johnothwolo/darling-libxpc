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

#include <os/transaction_private.h>
#include <os/object_private.h>
#import <Foundation/NSZone.h>
#import <xpc/xpc.h>
#import <xpc/private.h>
#import <xpc/util.h>
#import <libproc.h>

OS_OBJECT_NONLAZY_CLASS
@implementation OS_OBJECT_CLASS(os_transaction)

OS_OBJECT_NONLAZY_CLASS_LOAD

+ (instancetype)allocWithZone: (NSZone*)zone
{
	return (os_transaction_t)_os_object_alloc_realized([self class], sizeof(struct os_transaction_s));
}

- (instancetype)initWithName: (const char*)name
{
	// we CANNOT call `-[super init]`.
	// libdispatch makes `init` crash on `OS_object`s.
	return self;
}

@end

//
// C API
//

XPC_EXPORT
os_transaction_t os_transaction_create(const char* transaction_name) {
	return [[OS_OBJECT_CLASS(os_transaction) alloc] initWithName: transaction_name];
};

XPC_EXPORT
char* os_transaction_copy_description(os_transaction_t transaction) {
	xpc_stub();
	return NULL;
};

XPC_EXPORT
int os_transaction_needs_more_time(os_transaction_t transaction) {
	xpc_stub();
	return -1;
};

// these are technically not related to os_transaction,
// but the file is called `transaction.m`, so we'll dump them in here

typedef long os_once_t;
typedef os_once_t os_alloc_token_t;
struct _os_alloc_once_s {
    os_alloc_token_t once;
    void *ptr;
};

XPC_EXPORT
void xpc_transaction_begin(void) {
    xpc_globals_t globals = _xpc_globals();
    int64_t tcount_new; // eax

    os_unfair_lock_lock_with_options(&globals->lock, 0x10000);
    tcount_new = ++globals->transaction_count;
    if (!globals->transaction_enabled || tcount_new > 1)
        return;

    if (tcount_new < 1) xpc_abort("Underflow of transaction count.");
    (void)os_assumes_zero(proc_set_dirty(getpid(), true));
    os_unfair_lock_unlock(&globals->lock);
};

XPC_EXPORT
void xpc_transaction_enable(void) {
    xpc_globals_t globals = _xpc_globals();
    
    os_unfair_lock_lock_with_options(&globals->lock, 0x10000);
    if (!globals->transaction_enabled) {
        (void)os_assumes_zero(proc_track_dirty(getpid(), PROC_DIRTY_TRACK));
        globals->transaction_enabled = 1;
    }

    if (globals->transaction_count > 0) {
        (void)os_assumes_zero(proc_set_dirty(getpid(), true));
    }
    os_unfair_lock_unlock(&globals->lock);
};

XPC_EXPORT
void xpc_transaction_end(void) {
    xpc_globals_t globals = _xpc_globals();
    
    os_unfair_lock_lock_with_options(&globals->lock, 0x10000);
    globals->transaction_count--; // decrement transaction count
    
    if (globals->transaction_count < 0)
        xpc_abort("Underflow of transaction count.");
    
    if (globals->transaction_enabled || globals->transaction_count <= 0){
        (void)os_assumes_zero(proc_set_dirty(getpid(), false));
        if (globals->some_sort_of_exit_flag)
            _exit(0);
    }
    os_unfair_lock_unlock(&globals->lock);
};


// FIXME: complete this
int
xpc_interface_routine(int subsystem, int routine, xpc_object_t msg, xpc_object_t* out, int a5, char *a6)
{
    int r;
    xpc_object_t response;
    audit_token_t token;
    xpc_globals_t globals = _xpc_globals();
    
    r = 141;
    if (globals->_is_launchd_client || globals->_null_boostrap)
      return r;
    
    xpc_dictionary_set_uint64(msg, "subsystem", subsystem);
    xpc_dictionary_set_uint64(msg, "routine", routine);
    if (globals->_pre_exec_set)
      xpc_dictionary_set_bool(msg, "pre-exec", 1);
    r = xpc_pipe_routine((xpc_pipe_t)globals->bootstrap_pipe, msg, &response);
    if (!r) {
        r = xpc_dictionary_get_int64(msg, "error");
        xpc_dictionary_get_audit_token(response, &token);
//        if (token.pid!= 1 token.euid) { return 118; }
    }
    return r;
}

int
xpc_domain_routine(int routine, xpc_object_t msg, xpc_object_t *resp)
{
  return xpc_interface_routine(3, routine, msg, resp, 0, 0);
}

//
// private C API
//

XPC_EXPORT
void xpc_transaction_exit_clean(void) {
	xpc_stub();
};

XPC_EXPORT
void xpc_transaction_interrupt_clean_exit(void) {
	xpc_stub();
};

XPC_EXPORT
void xpc_transactions_enable(void) {
	xpc_stub();
};


// FIXME: move
#import <spawn.h>

// if null is passed to spawnattr the mach api is used instead

kern_return_t
xpc_set_exception_port(posix_spawnattr_t *spawnattr, mach_port_t new_port)
{
    int ret;
    task_t target_task;
    exception_mask_t exception_mask;
    thread_state_flavor_t tstate = 0;
    xpc_globals_t globals = _xpc_globals();

#if defined (__ppc__) || defined(__ppc64__)
    tstate = PPC_THREAD_STATE64;
#elif defined(__i386__) || defined(__x86_64__)
    tstate = x86_THREAD_STATE;
#elif defined(__arm__)
    tstate = ARM_THREAD_STATE;
#else
#error "unknown architecture"
#endif
    if (!globals->exit_to_corpse)
        exception_mask = EXC_MASK_RESOURCE | EXC_MASK_GUARD | EXC_MASK_CORPSE_NOTIFY;
    else
        exception_mask = EXC_MASK_CRASH | EXC_MASK_GUARD | EXC_MASK_RESOURCE;

    if (spawnattr == NULL)
        return task_set_exception_ports(target_task,
                                        exception_mask,
                                        new_port,
                                        EXCEPTION_STATE_IDENTITY | MACH_EXCEPTION_CODES,
                                        tstate);
    
    ret = posix_spawnattr_setexceptionports_np(spawnattr,
                                               exception_mask,
                                               new_port,
                                               EXCEPTION_STATE_IDENTITY | MACH_EXCEPTION_CODES,
                                               tstate);
    if (ret){
//        os_crash(os_assert_log(ret));
        xpc_abort(xpc_strerror(ret));
    }
    return 0;
}
