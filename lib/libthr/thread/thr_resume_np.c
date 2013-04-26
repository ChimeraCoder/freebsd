
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

__weak_reference(_pthread_resume_np, pthread_resume_np);
__weak_reference(_pthread_resume_all_np, pthread_resume_all_np);

static void resume_common(struct pthread *thread);

/* Resume a thread: */
int
_pthread_resume_np(pthread_t thread)
{
	struct pthread *curthread = _get_curthread();
	int ret;

	/* Add a reference to the thread: */
	if ((ret = _thr_find_thread(curthread, thread, /*include dead*/0)) == 0) {
		/* Lock the threads scheduling queue: */
		resume_common(thread);
		THR_THREAD_UNLOCK(curthread, thread);
	}
	return (ret);
}

void
_pthread_resume_all_np(void)
{
	struct pthread *curthread = _get_curthread();
	struct pthread *thread;
	int old_nocancel;

	old_nocancel = curthread->no_cancel;
	curthread->no_cancel = 1;
	_thr_suspend_all_lock(curthread);
	/* Take the thread list lock: */
	THREAD_LIST_RDLOCK(curthread);

	TAILQ_FOREACH(thread, &_thread_list, tle) {
		if (thread != curthread) {
			THR_THREAD_LOCK(curthread, thread);
			resume_common(thread);
			THR_THREAD_UNLOCK(curthread, thread);
		}
	}

	/* Release the thread list lock: */
	THREAD_LIST_UNLOCK(curthread);
	_thr_suspend_all_unlock(curthread);
	curthread->no_cancel = old_nocancel;
	_thr_testcancel(curthread);
}

static void
resume_common(struct pthread *thread)
{
	/* Clear the suspend flag: */
	thread->flags &= ~THR_FLAGS_NEED_SUSPEND;
	thread->cycle++;
	_thr_umtx_wake(&thread->cycle, 1, 0);
}