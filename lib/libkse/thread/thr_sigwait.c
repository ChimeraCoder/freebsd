
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

#include "namespace.h"
#include <signal.h>
#include <sys/param.h>
#include <sys/signalvar.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int	__sigtimedwait(const sigset_t *set, siginfo_t *info,
	    const struct timespec *timeout);
int	__sigwaitinfo(const sigset_t *set, siginfo_t *info);
int	__sigwait(const sigset_t *set, int *sig);
int	_sigtimedwait(const sigset_t *set, siginfo_t *info,
	    const struct timespec *timeout);
int	_sigwaitinfo(const sigset_t *set, siginfo_t *info);
int	_sigwait(const sigset_t *set, int *sig);

__weak_reference(__sigwait, sigwait);
__weak_reference(__sigtimedwait, sigtimedwait);
__weak_reference(__sigwaitinfo, sigwaitinfo);

static int
lib_sigtimedwait(const sigset_t *set, siginfo_t *info,
	const struct timespec *timeout)
{
	struct pthread	*curthread = _get_curthread();
	int		ret = 0;
	int		i;
	struct sigwait_data waitdata;
	sigset_t	waitset;
	kse_critical_t  crit;
	siginfo_t	siginfo;

	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM) {
		if (info == NULL)
			info = &siginfo;
		return (__sys_sigtimedwait(set, info, timeout));
	}

	/*
	 * Initialize the set of signals that will be waited on:
	 */
	waitset = *set;

	/* These signals can't be waited on. */
	SIGDELSET(waitset, SIGKILL);
	SIGDELSET(waitset, SIGSTOP);

	/*
	 * POSIX says that the _application_ must explicitly install
	 * a dummy handler for signals that are SIG_IGN in order
	 * to sigwait on them. Note that SIG_IGN signals are left in
	 * the mask because a subsequent sigaction could enable an
	 * ignored signal.
	 */

	crit = _kse_critical_enter();
	KSE_SCHED_LOCK(curthread->kse, curthread->kseg);
	for (i = 1; i <= _SIG_MAXSIG; ++i) {
		if (SIGISMEMBER(waitset, i) &&
		    SIGISMEMBER(curthread->sigpend, i)) {
			SIGDELSET(curthread->sigpend, i);
			siginfo = curthread->siginfo[i - 1];
			KSE_SCHED_UNLOCK(curthread->kse,
				curthread->kseg);
			_kse_critical_leave(crit);
			ret = i;
			goto OUT;
		}
	}
	curthread->timeout = 0;
	curthread->interrupted = 0;
	_thr_set_timeout(timeout);
	/* Wait for a signal: */
	siginfo.si_signo = 0;
	waitdata.waitset = &waitset;
	waitdata.siginfo = &siginfo;
	curthread->data.sigwait = &waitdata;
	THR_SET_STATE(curthread, PS_SIGWAIT);
	_thr_sched_switch_unlocked(curthread);
	/*
	 * Return the signal number to the caller:
	 */
	if (siginfo.si_signo > 0) {
		ret = siginfo.si_signo;
	} else {
		if (curthread->interrupted)
			errno = EINTR;
		else if (curthread->timeout)
			errno = EAGAIN;
		ret = -1;
	}
	curthread->timeout = 0;
	curthread->interrupted = 0;
	/*
	 * Probably unnecessary, but since it's in a union struct
	 * we don't know how it could be used in the future.
	 */
	curthread->data.sigwait = NULL;

OUT:
	if (ret > 0 && info != NULL)
		*info = siginfo;

	return (ret);
}

int
__sigtimedwait(const sigset_t *set, siginfo_t *info,
	const struct timespec * timeout)
{
	struct pthread	*curthread = _get_curthread();
	int ret;

	_thr_cancel_enter(curthread);
	ret = lib_sigtimedwait(set, info, timeout);
	_thr_cancel_leave(curthread, 1);
	return (ret);
}

int _sigtimedwait(const sigset_t *set, siginfo_t *info,
	const struct timespec * timeout)
{
	return lib_sigtimedwait(set, info, timeout);
}

int
__sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	struct pthread	*curthread = _get_curthread();
	int ret;

	_thr_cancel_enter(curthread);
	ret = lib_sigtimedwait(set, info, NULL);
	_thr_cancel_leave(curthread, 1);
	return (ret);
}

int
_sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	return lib_sigtimedwait(set, info, NULL);
}

int
__sigwait(const sigset_t *set, int *sig)
{
	struct pthread	*curthread = _get_curthread();
	int ret;

	_thr_cancel_enter(curthread);
	ret = lib_sigtimedwait(set, NULL, NULL);
	if (ret > 0) {
		*sig = ret;
		ret = 0;
	} else {
		ret = errno;
	}
	_thr_cancel_leave(curthread, 1);
	return (ret);
}

int
_sigwait(const sigset_t *set, int *sig)
{
	int ret;

	ret = lib_sigtimedwait(set, NULL, NULL);
	if (ret > 0) {
		*sig = ret;
		ret = 0;
	} else {
		ret = errno;
	}
	return (ret);
}