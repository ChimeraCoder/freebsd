
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
#include <errno.h>
#include <sys/param.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_setschedparam, pthread_setschedparam);

int
_pthread_setschedparam(pthread_t pthread, int policy, 
	const struct sched_param *param)
{
	struct pthread *curthread = _get_curthread();
	int	in_syncq;
	int	in_readyq = 0;
	int	old_prio;
	int	ret = 0;

	if ((param == NULL) || (policy < SCHED_FIFO) || (policy > SCHED_RR)) {
		/* Return an invalid argument error: */
		ret = EINVAL;
	} else if ((param->sched_priority < THR_MIN_PRIORITY) ||
	    (param->sched_priority > THR_MAX_PRIORITY)) {
		/* Return an unsupported value error. */
		ret = ENOTSUP;

	/* Find the thread in the list of active threads: */
	} else if ((ret = _thr_ref_add(curthread, pthread, /*include dead*/0))
	    == 0) {
		/*
		 * Lock the threads scheduling queue while we change
		 * its priority:
		 */
		THR_SCHED_LOCK(curthread, pthread);
		if ((pthread->state == PS_DEAD) ||
		    (pthread->state == PS_DEADLOCK) ||
		    ((pthread->flags & THR_FLAGS_EXITING) != 0)) {
			THR_SCHED_UNLOCK(curthread, pthread);
			_thr_ref_delete(curthread, pthread);
			return (ESRCH);
		}
		in_syncq = pthread->sflags & THR_FLAGS_IN_SYNCQ;

		/* Set the scheduling policy: */
		pthread->attr.sched_policy = policy;

		if (param->sched_priority ==
		    THR_BASE_PRIORITY(pthread->base_priority))
			/*
			 * There is nothing to do; unlock the threads
			 * scheduling queue.
			 */
			THR_SCHED_UNLOCK(curthread, pthread);
		else {
			/*
			 * Remove the thread from its current priority
			 * queue before any adjustments are made to its
			 * active priority:
			 */
			old_prio = pthread->active_priority;
			if ((pthread->flags & THR_FLAGS_IN_RUNQ) != 0) {
				in_readyq = 1;
				THR_RUNQ_REMOVE(pthread);
			}

			/* Set the thread base priority: */
			pthread->base_priority &=
			    (THR_SIGNAL_PRIORITY | THR_RT_PRIORITY);
			pthread->base_priority = param->sched_priority;

			/* Recalculate the active priority: */
			pthread->active_priority = MAX(pthread->base_priority,
			    pthread->inherited_priority);

			if (in_readyq) {
				if ((pthread->priority_mutex_count > 0) &&
				    (old_prio > pthread->active_priority)) {
					/*
					 * POSIX states that if the priority is
					 * being lowered, the thread must be
					 * inserted at the head of the queue for
					 * its priority if it owns any priority
					 * protection or inheritence mutexes.
					 */
					THR_RUNQ_INSERT_HEAD(pthread);
				}
				else
					THR_RUNQ_INSERT_TAIL(pthread);
			}

			/* Unlock the threads scheduling queue: */
			THR_SCHED_UNLOCK(curthread, pthread);

			/*
			 * Check for any mutex priority adjustments.  This
			 * includes checking for a priority mutex on which
			 * this thread is waiting.
			 */
			_mutex_notify_priochange(curthread, pthread, in_syncq);
		}
		_thr_ref_delete(curthread, pthread);
	}
	return (ret);
}