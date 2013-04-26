
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
#include <machine/atomic.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_detach, pthread_detach);

int
_pthread_detach(pthread_t pthread)
{
	struct pthread *curthread = _get_curthread();
	struct kse_mailbox *kmbx = NULL;
	struct pthread *joiner;
	int rval = 0;

	/* Check for invalid calling parameters: */
	if (pthread == NULL || pthread->magic != THR_MAGIC)
		/* Return an invalid argument error: */
		rval = EINVAL;

	else if ((rval = _thr_ref_add(curthread, pthread,
	    /*include dead*/1)) != 0) {
		/* Return an error: */
	}

	/* Check if the thread is already detached: */
	else if ((pthread->attr.flags & PTHREAD_DETACHED) != 0) {
		/* Return an error: */
		_thr_ref_delete(curthread, pthread);
		rval = EINVAL;
	} else {
		/* Lock the detached thread: */
		THR_SCHED_LOCK(curthread, pthread);

		/* Flag the thread as detached: */
		pthread->attr.flags |= PTHREAD_DETACHED;

		/* Retrieve any joining thread and remove it: */
		joiner = pthread->joiner;
		if ((joiner != NULL) && (joiner->kseg == pthread->kseg)) {
			/*
			 * We already own the scheduler lock for the joiner.
			 * Take advantage of that and make the joiner runnable.
			 */
			if (joiner->join_status.thread == pthread) {
				/*
				 * Set the return value for the woken thread:
				 */
				joiner->join_status.error = ESRCH;
				joiner->join_status.ret = NULL;
				joiner->join_status.thread = NULL;

				kmbx = _thr_setrunnable_unlocked(joiner);
			}
			joiner = NULL;
		}
		THR_SCHED_UNLOCK(curthread, pthread);
		/* See if there is a thread waiting in pthread_join(): */
		if ((joiner != NULL) &&
		    (_thr_ref_add(curthread, joiner, 0) == 0)) {
			/* Lock the joiner before fiddling with it. */
			THR_SCHED_LOCK(curthread, joiner);
			if (joiner->join_status.thread == pthread) {
				/*
				 * Set the return value for the woken thread:
				 */
				joiner->join_status.error = ESRCH;
				joiner->join_status.ret = NULL;
				joiner->join_status.thread = NULL;

				kmbx = _thr_setrunnable_unlocked(joiner);
			}
			THR_SCHED_UNLOCK(curthread, joiner);
			_thr_ref_delete(curthread, joiner);
		}
		_thr_ref_delete(curthread, pthread);
		if (kmbx != NULL)
			kse_wakeup(kmbx);
	}

	/* Return the completion status: */
	return (rval);
}