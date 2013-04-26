
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <errno.h>
#include <link.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include "libc_private.h"

extern int __sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
    void *newp, size_t newlen);

long __stack_chk_guard[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static void __guard_setup(void) __attribute__((__constructor__, __used__));
static void __fail(const char *);
void __stack_chk_fail(void);
void __chk_fail(void);

/*LINTED used*/
static void
__guard_setup(void)
{
	int mib[2];
	size_t len;
	int error;

	if (__stack_chk_guard[0] != 0)
		return;
	error = _elf_aux_info(AT_CANARY, __stack_chk_guard,
	    sizeof(__stack_chk_guard));
	if (error == 0 && __stack_chk_guard[0] != 0)
		return;

	mib[0] = CTL_KERN;
	mib[1] = KERN_ARND;

	len = sizeof(__stack_chk_guard);
	if (__sysctl(mib, 2, __stack_chk_guard, &len, NULL, 0) == -1 ||
	    len != sizeof(__stack_chk_guard)) {
		/* If sysctl was unsuccessful, use the "terminator canary". */
		((unsigned char *)(void *)__stack_chk_guard)[0] = 0;
		((unsigned char *)(void *)__stack_chk_guard)[1] = 0;
		((unsigned char *)(void *)__stack_chk_guard)[2] = '\n';
		((unsigned char *)(void *)__stack_chk_guard)[3] = 255;
	}
}

/*ARGSUSED*/
static void
__fail(const char *msg)
{
	struct sigaction sa;
	sigset_t mask;

	/* Immediately block all signal handlers from running code */
	(void)sigfillset(&mask);
	(void)sigdelset(&mask, SIGABRT);
	(void)sigprocmask(SIG_BLOCK, &mask, NULL);

	/* This may fail on a chroot jail... */
	syslog(LOG_CRIT, "%s", msg);

	(void)memset(&sa, 0, sizeof(sa));
	(void)sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	(void)sigaction(SIGABRT, &sa, NULL);
	(void)kill(getpid(), SIGABRT);
	_exit(127);
}

void
__stack_chk_fail(void)
{
	__fail("stack overflow detected; terminated");
}

void
__chk_fail(void)
{
	__fail("buffer overflow detected; terminated");
}

#ifndef PIC
__weak_reference(__stack_chk_fail, __stack_chk_fail_local);
#endif