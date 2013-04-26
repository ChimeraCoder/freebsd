
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

int	_pthread_resume_np(pthread_t thread);
void	_pthread_resume_all_np(void);

static struct kse_mailbox *resume_common(struct pthread *);

__weak_reference(_pthread_resume_np, pthread_resume_np);
__weak_reference(_pthread_resume_all_np, pthread_resume_all_np);


/* Resume a thread: */
int
_pthread_resume_np(pthread_t thread)
{
	struct pthread *curthread = _get_curthread();
	struct kse_mailbox *kmbx;
	int ret;

	/* Add a reference to the thread: */
	if ((ret = _thr_ref_add(curthread, thread, /*include dead*/0)) == 0) {
		/* Lock the threads scheduling queue: */
		THR_SCHED_LOCK(curthread, thread);
		kmbx = resume_common(thread);
		THR_SCHED_UNLOCK(curthread, thread);
		_thr_ref_delete(curthread, thread);
		if (kmbx != NULL)
			kse_wakeup(kmbx);
	}
	return (ret);
}

void
_pthread_resume_all_np(void)
{
	struct pthread *curthread = _get_curthread();
	struct pthread *thread;
	struct kse_mailbox *kmbx;
	kse_critical_t crit;

	/* Take the thread list lock: */
	crit = _kse_critical_enter();
	KSE_LOCK_ACQUIRE(curthread->kse, &_thread_list_lock);

	TAILQ_FOREACH(thread, &_thread_list, tle) {
		if (thread != curthread) {
			THR_SCHED_LOCK(curthread, thread);
			kmbx = resume_common(thread);
			THR_SCHED_UNLOCK(curthread, thread);
			if (kmbx != NULL)
				kse_wakeup(kmbx);
		}
	}

	/* Release the thread list lock: */
	KSE_LOCK_RELEASE(curthread->kse, &_thread_list_lock);
	_kse_critical_leave(crit);
}

static struct kse_mailbox *
resume_common(struct pthread *thread)
{
	/* Clear the suspend flag: */
	thread->flags &= ~THR_FLAGS_SUSPENDED;

	/*
	 * If the thread's state is suspended, that means it is
	 * now runnable but not in any scheduling queue.  Set the
	 * state to running and insert it into the run queue.
	 */
	if (thread->state == PS_SUSPENDED)
		return (_thr_setrunnable_unlocked(thread));
	else
		return (NULL);
}