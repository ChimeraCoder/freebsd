
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

int	_pthread_timedjoin_np(pthread_t pthread, void **thread_return,
	const struct timespec *abstime);
static int join_common(pthread_t, void **, const struct timespec *);

__weak_reference(_pthread_join, pthread_join);
__weak_reference(_pthread_timedjoin_np, pthread_timedjoin_np);

static void backout_join(void *arg)
{
	struct pthread *pthread = (struct pthread *)arg;
	struct pthread *curthread = _get_curthread();

	THR_THREAD_LOCK(curthread, pthread);
	pthread->joiner = NULL;
	THR_THREAD_UNLOCK(curthread, pthread);
}

int
_pthread_join(pthread_t pthread, void **thread_return)
{
	return (join_common(pthread, thread_return, NULL));
}

int
_pthread_timedjoin_np(pthread_t pthread, void **thread_return,
	const struct timespec *abstime)
{
	if (abstime == NULL || abstime->tv_sec < 0 || abstime->tv_nsec < 0 ||
	    abstime->tv_nsec >= 1000000000)
		return (EINVAL);

	return (join_common(pthread, thread_return, abstime));
}

/*
 * Cancellation behavior:
 *   if the thread is canceled, joinee is not recycled.
 */
static int
join_common(pthread_t pthread, void **thread_return,
	const struct timespec *abstime)
{
	struct pthread *curthread = _get_curthread();
	struct timespec ts, ts2, *tsp;
	void *tmp;
	long tid;
	int ret = 0;

	if (pthread == NULL)
		return (EINVAL);

	if (pthread == curthread)
		return (EDEADLK);

	if ((ret = _thr_find_thread(curthread, pthread, 1)) != 0)
		return (ESRCH);

	if ((pthread->flags & THR_FLAGS_DETACHED) != 0) {
		ret = EINVAL;
	} else if (pthread->joiner != NULL) {
		/* Multiple joiners are not supported. */
		ret = ENOTSUP;
	}
	if (ret) {
		THR_THREAD_UNLOCK(curthread, pthread);
		return (ret);
	}
	/* Set the running thread to be the joiner: */
	pthread->joiner = curthread;

	THR_THREAD_UNLOCK(curthread, pthread);

	THR_CLEANUP_PUSH(curthread, backout_join, pthread);
	_thr_cancel_enter(curthread);

	tid = pthread->tid;
	while (pthread->tid != TID_TERMINATED) {
		_thr_testcancel(curthread);
		if (abstime != NULL) {
			clock_gettime(CLOCK_REALTIME, &ts);
			TIMESPEC_SUB(&ts2, abstime, &ts);
			if (ts2.tv_sec < 0) {
				ret = ETIMEDOUT;
				break;
			}
			tsp = &ts2;
		} else
			tsp = NULL;
		ret = _thr_umtx_wait(&pthread->tid, tid, tsp);
		if (ret == ETIMEDOUT)
			break;
	}

	_thr_cancel_leave(curthread, 0);
	THR_CLEANUP_POP(curthread, 0);

	if (ret == ETIMEDOUT) {
		THR_THREAD_LOCK(curthread, pthread);
		pthread->joiner = NULL;
		THR_THREAD_UNLOCK(curthread, pthread);
	} else {
		ret = 0;
		tmp = pthread->ret;
		THR_THREAD_LOCK(curthread, pthread);
		pthread->flags |= THR_FLAGS_DETACHED;
		pthread->joiner = NULL;
		_thr_try_gc(curthread, pthread); /* thread lock released */

		if (thread_return != NULL)
			*thread_return = tmp;
	}
	return (ret);
}