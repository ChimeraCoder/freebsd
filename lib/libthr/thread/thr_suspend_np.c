
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
#include <pthread_np.h>
#include "un-namespace.h"

#include "thr_private.h"

static int suspend_common(struct pthread *, struct pthread *,
		int);

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
		THR_THREAD_LOCK(curthread, thread);
		suspend_common(curthread, thread, 1);
		/* Unlock the threads scheduling queue: */
		THR_THREAD_UNLOCK(curthread, thread);

		/* Don't forget to remove the reference: */
		_thr_ref_delete(curthread, thread);
	}
	return (ret);
}

void
_thr_suspend_all_lock(struct pthread *curthread)
{
	int old;

	THR_LOCK_ACQUIRE(curthread, &_suspend_all_lock);
	while (_single_thread != NULL) {
		old = _suspend_all_cycle;
		_suspend_all_waiters++;
		THR_LOCK_RELEASE(curthread, &_suspend_all_lock);
		_thr_umtx_wait_uint(&_suspend_all_cycle, old, NULL, 0);
		THR_LOCK_ACQUIRE(curthread, &_suspend_all_lock);
		_suspend_all_waiters--;
	}
	_single_thread = curthread;
	THR_LOCK_RELEASE(curthread, &_suspend_all_lock);
}

void
_thr_suspend_all_unlock(struct pthread *curthread)
{

	THR_LOCK_ACQUIRE(curthread, &_suspend_all_lock);
	_single_thread = NULL;
	if (_suspend_all_waiters != 0) {
		_suspend_all_cycle++;
		_thr_umtx_wake(&_suspend_all_cycle, INT_MAX, 0);
	}
	THR_LOCK_RELEASE(curthread, &_suspend_all_lock);
}

void
_pthread_suspend_all_np(void)
{
	struct pthread *curthread = _get_curthread();
	struct pthread *thread;
	int old_nocancel;
	int ret;

	old_nocancel = curthread->no_cancel;
	curthread->no_cancel = 1;
	_thr_suspend_all_lock(curthread);
	THREAD_LIST_RDLOCK(curthread);
	TAILQ_FOREACH(thread, &_thread_list, tle) {
		if (thread != curthread) {
			THR_THREAD_LOCK(curthread, thread);
			if (thread->state != PS_DEAD &&
	      		   !(thread->flags & THR_FLAGS_SUSPENDED))
			    thread->flags |= THR_FLAGS_NEED_SUSPEND;
			THR_THREAD_UNLOCK(curthread, thread);
		}
	}
	thr_kill(-1, SIGCANCEL);

restart:
	TAILQ_FOREACH(thread, &_thread_list, tle) {
		if (thread != curthread) {
			/* First try to suspend the thread without waiting */
			THR_THREAD_LOCK(curthread, thread);
			ret = suspend_common(curthread, thread, 0);
			if (ret == 0) {
				THREAD_LIST_UNLOCK(curthread);
				/* Can not suspend, try to wait */
				THR_REF_ADD(curthread, thread);
				suspend_common(curthread, thread, 1);
				THR_REF_DEL(curthread, thread);
				_thr_try_gc(curthread, thread);
				/* thread lock released */

				THREAD_LIST_RDLOCK(curthread);
				/*
				 * Because we were blocked, things may have
				 * been changed, we have to restart the
				 * process.
				 */
				goto restart;
			}
			THR_THREAD_UNLOCK(curthread, thread);
		}
	}
	THREAD_LIST_UNLOCK(curthread);
	_thr_suspend_all_unlock(curthread);
	curthread->no_cancel = old_nocancel;
	_thr_testcancel(curthread);
}

static int
suspend_common(struct pthread *curthread, struct pthread *thread,
	int waitok)
{
	uint32_t tmp;

	while (thread->state != PS_DEAD &&
	      !(thread->flags & THR_FLAGS_SUSPENDED)) {
		thread->flags |= THR_FLAGS_NEED_SUSPEND;
		/* Thread is in creation. */
		if (thread->tid == TID_TERMINATED)
			return (1);
		tmp = thread->cycle;
		_thr_send_sig(thread, SIGCANCEL);
		THR_THREAD_UNLOCK(curthread, thread);
		if (waitok) {
			_thr_umtx_wait_uint(&thread->cycle, tmp, NULL, 0);
			THR_THREAD_LOCK(curthread, thread);
		} else {
			THR_THREAD_LOCK(curthread, thread);
			return (0);
		}
	}

	return (1);
}