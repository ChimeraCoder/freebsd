
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)sigcompat.c	8.1 (Berkeley) 6/2/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "un-namespace.h"
#include "libc_private.h"

int
sigvec(signo, sv, osv)
	int signo;
	struct sigvec *sv, *osv;
{
	struct sigaction sa, osa;
	struct sigaction *sap, *osap;
	int ret;

	if (sv != NULL) {
		sa.sa_handler = sv->sv_handler;
		sa.sa_flags = sv->sv_flags ^ SV_INTERRUPT;
		sigemptyset(&sa.sa_mask);
		sa.sa_mask.__bits[0] = sv->sv_mask;
		sap = &sa;
	} else
		sap = NULL;
	osap = osv != NULL ? &osa : NULL;
	ret = _sigaction(signo, sap, osap);
	if (ret == 0 && osv != NULL) {
		osv->sv_handler = osa.sa_handler;
		osv->sv_flags = osa.sa_flags ^ SV_INTERRUPT;
		osv->sv_mask = osa.sa_mask.__bits[0];
	}
	return (ret);
}

int
sigsetmask(mask)
	int mask;
{
	sigset_t set, oset;
	int n;

	sigemptyset(&set);
	set.__bits[0] = mask;
	n = _sigprocmask(SIG_SETMASK, &set, &oset);
	if (n)
		return (n);
	return (oset.__bits[0]);
}

int
sigblock(mask)
	int mask;
{
	sigset_t set, oset;
	int n;

	sigemptyset(&set);
	set.__bits[0] = mask;
	n = _sigprocmask(SIG_BLOCK, &set, &oset);
	if (n)
		return (n);
	return (oset.__bits[0]);
}

int
sigpause(int mask)
{
	sigset_t set;

	sigemptyset(&set);
	set.__bits[0] = mask;
	return (_sigsuspend(&set));
}

int
xsi_sigpause(int sig)
{
	sigset_t set;

	if (_sigprocmask(SIG_BLOCK, NULL, &set) == -1)
		return (-1);
	if (sigdelset(&set, sig) == -1)
		return (-1);
	return (_sigsuspend(&set));
}

int
sighold(int sig)
{
	sigset_t set;

	sigemptyset(&set);
	if (sigaddset(&set, sig) == -1)
		return (-1);
	return (_sigprocmask(SIG_BLOCK, &set, NULL));
}

int
sigignore(int sig)
{
	struct sigaction sa;

	bzero(&sa, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	return (_sigaction(sig, &sa, NULL));
}

int
sigrelse(int sig)
{
	sigset_t set;

	sigemptyset(&set);
	if (sigaddset(&set, sig) == -1)
		return (-1);
	return (_sigprocmask(SIG_UNBLOCK, &set, NULL));
}

void
(*sigset(int sig, void (*disp)(int)))(int)
{
	sigset_t set, pset;
	struct sigaction sa, psa;

	sigemptyset(&set);
	if (sigaddset(&set, sig) == -1)
		return (SIG_ERR);
	if (_sigprocmask(SIG_BLOCK, NULL, &pset) == -1)
		return (SIG_ERR);
	if ((__sighandler_t *)disp == SIG_HOLD) {
		if (_sigprocmask(SIG_BLOCK, &set, &pset) == -1)
			return (SIG_ERR);
		if (sigismember(&pset, sig))
			return (SIG_HOLD);
		else {
			if (_sigaction(sig, NULL, &psa) == -1)
				return (SIG_ERR);
			return (psa.sa_handler);
		}
	} else {
		if (_sigprocmask(SIG_UNBLOCK, &set, &pset) == -1)
			return (SIG_ERR);
	}

	bzero(&sa, sizeof(sa));
	sa.sa_handler = disp;
	if (_sigaction(sig, &sa, &psa) == -1)
		return (SIG_ERR);
	if (sigismember(&pset, sig))
		return (SIG_HOLD);
	else
		return (psa.sa_handler);
}