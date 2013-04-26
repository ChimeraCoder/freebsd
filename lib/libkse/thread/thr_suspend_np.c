
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
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int	_pthread_suspend_np(pthread_t thread);
void	_pthread_suspend_all_np(void);

static void suspend_common(struct pthread *thread);

__weak_reference(_pthread_suspend_np, pthread_suspend_np);
__weak_reference(_pthread_suspend_all_np, pthread_suspend_all_np);

/* Suspend a thread: */
int
_pthread_suspend_np(pthread_t thread)
{
	struct pthread *curthread = _get_curthread();
	int ret;

	/* Suspending the current thread doesn't make sense. */
	if (thread == _get_curthread())
		ret = EDEADLK;

	/* Add a reference to the thread: */
	else if ((ret = _thr_ref_add(curthread, thread, /*include dead*/0))
	    == 0) {
		/* Lock the threads scheduling queue: */
		THR_SCHED_LOCK(curthread, thread);
		suspend_common(thread);
		/* Unlock the threads scheduling queue: */
		THR_SCHED_UNLOCK(curthread, thread);

		/* Don't forget to remove the reference: */
		_thr_ref_delete(curthread, thread);
	}
	return (ret);
}

void
_pthread_suspend_all_np(void)
{
	struct pthread	*curthread = _get_curthread();
	struct pthread	*thread;
	kse_critical_t crit;

	/* Take the thread list lock: */
	crit = _kse_critical_enter();
	KSE_LOCK_ACQUIRE(curthread->kse, &_thread_list_lock);

	TAILQ_FOREACH(thread, &_thread_list, tle) {
		if (thread != curthread) {
			THR_SCHED_LOCK(curthread, thread);
			suspend_common(thread);
			THR_SCHED_UNLOCK(curthread, thread);
		}
	}

	/* Release the thread list lock: */
	KSE_LOCK_RELEASE(curthread->kse, &_thread_list_lock);
	_kse_critical_leave(crit);
}

void
suspend_common(struct pthread *thread)
{
	if ((thread->state != PS_DEAD) &&
	    (thread->state != PS_DEADLOCK) &&
	    ((thread->flags & THR_FLAGS_EXITING) == 0)) {
		thread->flags |= THR_FLAGS_SUSPENDED;
		if ((thread->flags & THR_FLAGS_IN_RUNQ) != 0) {
			THR_RUNQ_REMOVE(thread);
			THR_SET_STATE(thread, PS_SUSPENDED);
		}
#ifdef NOT_YET
		if ((thread->attr.flags & PTHREAD_SCOPE_SYSTEM) != 0)
			/* ??? */
#endif
	}
}