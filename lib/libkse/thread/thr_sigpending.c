
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
#include <sys/param.h>
#include <sys/types.h>
#include <sys/signalvar.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int	_sigpending(sigset_t *set);

__weak_reference(_sigpending, sigpending);

int
_sigpending(sigset_t *set)
{
	struct pthread *curthread = _get_curthread();
	kse_critical_t crit;
	sigset_t sigset;
	int ret = 0;

	/* Check for a null signal set pointer: */
	if (set == NULL) {
		/* Return an invalid argument: */
		ret = EINVAL;
	}
	else {
		if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM)
			return (__sys_sigpending(set));

		crit = _kse_critical_enter();
		KSE_SCHED_LOCK(curthread->kse, curthread->kseg);
		sigset = curthread->sigpend;
		KSE_SCHED_UNLOCK(curthread->kse, curthread->kseg);
		KSE_LOCK_ACQUIRE(curthread->kse, &_thread_signal_lock);
		SIGSETOR(sigset, _thr_proc_sigpending);
		KSE_LOCK_RELEASE(curthread->kse, &_thread_signal_lock);
		_kse_critical_leave(crit);
		*set = sigset;
	}
	/* Return the completion status: */
	return (ret);
}