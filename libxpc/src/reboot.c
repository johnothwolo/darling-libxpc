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

#import <errno.h>
#import <reboot2.h>
#import <xpc/xpc.h>

#define bittest64(__a, __b)                                             \
({                                                                      \
        unsigned char *___a = (unsigned char*)(__a);                    \
        int64_t ___b = (uint64_t)(__b);                                 \
        (___a[___b >> 3] & (unsigned char)(1 << (___b & 7))) != 0;      \
})

XPC_EXPORT
void *reboot3(uint64_t flags, const char *fmt, ...)
{
    xpc_object_t response;
    kern_return_t ret = 0;
    char purpose[8192] = {0};
    va_list ap;
    
    xpc_object_t request = xpc_dictionary_create(0, 0, 0);
    xpc_dictionary_set_uint64(request, "type", 1); // The type should be task_special_port_t.
    xpc_dictionary_set_uint64(request, "handle", 0);
    xpc_dictionary_set_uint64(request, "flags", flags);

    if (fmt != NULL){
        va_start(ap, fmt);
        vsprintf(purpose, fmt, ap);
        va_end(ap);
        xpc_dictionary_set_uint64(request, "userreboot_purpose", purpose);
    }
    ret = xpc_domain_routine(821, request, &response);
    
    if (!ret){
        xpc_release(response);
        // Apple doesn't release this.
        // I understand the system's gonna shutdown, but isn't it bad coding practice?
    }
    
    xpc_release(request);
    return ret;
}

XPC_EXPORT
void *reboot2(uint64_t flags)
{
    uint64_t ret = reboot3(flags, NULL);
    if (ret){
      errno = ret;
      ret = reboot2;
    }
    return ret;
}

