
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
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_sigmask, pthread_sigmask);

int
_pthread_sigmask(int how, const sigset_t *set, sigset_t *oset)
{
	struct pthread *curthread = _get_curthread();
	sigset_t oldset, newset;
	int ret;

	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM) {
		ret = __sys_sigprocmask(how, set, oset);
		if (ret != 0)
			ret = errno;
		/* Get a fresh copy */
		__sys_sigprocmask(SIG_SETMASK, NULL, &curthread->sigmask);
		return (ret);
	}

	if (set)
		newset = *set;

	THR_SCHED_LOCK(curthread, curthread);

	ret = 0;
	if (oset != NULL)
		/* Return the current mask: */
		oldset = curthread->sigmask;

	/* Check if a new signal set was provided by the caller: */
	if (set != NULL) {
		/* Process according to what to do: */
		switch (how) {
		/* Block signals: */
		case SIG_BLOCK:
			/* Add signals to the existing mask: */
			SIGSETOR(curthread->sigmask, newset);
			break;

		/* Unblock signals: */
		case SIG_UNBLOCK:
			/* Clear signals from the existing mask: */
			SIGSETNAND(curthread->sigmask, newset);
			break;

		/* Set the signal process mask: */
		case SIG_SETMASK:
			/* Set the new mask: */
			curthread->sigmask = newset;
			break;

		/* Trap invalid actions: */
		default:
			/* Return an invalid argument: */
			ret = EINVAL;
			break;
		}
		SIG_CANTMASK(curthread->sigmask);
		THR_SCHED_UNLOCK(curthread, curthread);

		/*
		 * Run down any pending signals:
		 */
		if (ret == 0)
		    _thr_sig_check_pending(curthread);
	} else
		THR_SCHED_UNLOCK(curthread, curthread);

	if (ret == 0 && oset != NULL)
		*oset = oldset;
	return (ret);
}