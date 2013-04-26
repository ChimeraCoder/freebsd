
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
#include <sys/types.h>
#include <sys/signalvar.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "un-namespace.h"
#include "thr_private.h"

int	__sigsuspend(const sigset_t * set);

__weak_reference(__sigsuspend, sigsuspend);

int
_sigsuspend(const sigset_t *set)
{
	struct pthread	*curthread = _get_curthread();
	sigset_t	oldmask, newmask, tempset;
	int             ret = -1;

	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM)
		return (__sys_sigsuspend(set));

	/* Check if a new signal set was provided by the caller: */
	if (set != NULL) {
		newmask = *set;
		SIG_CANTMASK(newmask);
		THR_LOCK_SWITCH(curthread);

		/* Save current sigmask: */
		oldmask = curthread->sigmask;
		curthread->oldsigmask = &oldmask;

		/* Change the caller's mask: */
		curthread->sigmask = newmask;
		tempset = curthread->sigpend;
		SIGSETNAND(tempset, newmask);
		if (SIGISEMPTY(tempset)) {
			THR_SET_STATE(curthread, PS_SIGSUSPEND);
			/* Wait for a signal: */
			_thr_sched_switch_unlocked(curthread);
		} else {
			curthread->check_pending = 1;
			THR_UNLOCK_SWITCH(curthread);
			/* check pending signal I can handle: */
			_thr_sig_check_pending(curthread);
		}
		if ((curthread->cancelflags & THR_CANCELLING) != 0)
			curthread->oldsigmask = NULL;
		else {
			THR_ASSERT(curthread->oldsigmask == NULL,
		 	          "oldsigmask is not cleared");
		}

		/* Always return an interrupted error: */
		errno = EINTR;
	} else {
		/* Return an invalid argument error: */
		errno = EINVAL;
	}

	/* Return the completion status: */
	return (ret);
}

int
__sigsuspend(const sigset_t * set)
{
	struct pthread *curthread = _get_curthread();
	int		ret;

	_thr_cancel_enter(curthread);
	ret = _sigsuspend(set);
	_thr_cancel_leave(curthread, 1);

	return (ret);
}