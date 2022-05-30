/*-
 * Copyright (c) 2004-2009 Apple Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $P4: //depot/projects/trustedbsd/openbsm/libbsm/bsm_wrappers.c#31 $
 */

#ifdef __APPLE__
#define	_SYS_AUDIT_H		/* Prevent include of sys/audit.h. */
#endif

#include <sys/param.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <sys/queue.h>		/* Our bsm/audit.h doesn't include queue.h. */
#endif

#include <sys/sysctl.h>

#include <bsm/libbsm.h>

#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>


#ifdef __APPLE__
void __attribute__((weak))
audit_token_to_au32(audit_token_t atoken, uid_t *auidp, uid_t *euidp,
    gid_t *egidp, uid_t *ruidp, gid_t *rgidp, pid_t *pidp, au_asid_t *asidp,
    au_tid_t *tidp)
{

	if (auidp != NULL)
		*auidp = (uid_t)atoken.val[0];
	if (euidp != NULL)
		*euidp = (uid_t)atoken.val[1];
	if (egidp != NULL)
		*egidp = (gid_t)atoken.val[2];
	if (ruidp != NULL)
		*ruidp = (uid_t)atoken.val[3];
	if (rgidp != NULL)
		*rgidp = (gid_t)atoken.val[4];
	if (pidp != NULL)
		*pidp = (pid_t)atoken.val[5];
	if (asidp != NULL)
		*asidp = (au_asid_t)atoken.val[6];
	if (tidp != NULL) {
		// audit_set_terminal_host(&tidp->machine);
		tidp->port = (dev_t)atoken.val[7];
	}
}
#endif /* !__APPLE__ */

