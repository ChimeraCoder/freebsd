
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
#include <pthread.h>
#include "un-namespace.h"

#include "thr_private.h"

__weak_reference(_pthread_cancel, pthread_cancel);
__weak_reference(_pthread_setcancelstate, pthread_setcancelstate);
__weak_reference(_pthread_setcanceltype, pthread_setcanceltype);
__weak_reference(_pthread_testcancel, pthread_testcancel);

static inline void
testcancel(struct pthread *curthread)
{
	if (__predict_false(SHOULD_CANCEL(curthread) &&
	    !THR_IN_CRITICAL(curthread)))
		_pthread_exit(PTHREAD_CANCELED);
}

void
_thr_testcancel(struct pthread *curthread)
{
	testcancel(curthread);
}

int
_pthread_cancel(pthread_t pthread)
{
	struct pthread *curthread = _get_curthread();
	int ret;

	/*
	 * POSIX says _pthread_cancel should be async cancellation safe.
	 * _thr_find_thread and THR_THREAD_UNLOCK will enter and leave critical
	 * region automatically.
	 */
	if ((ret = _thr_find_thread(curthread, pthread, 0)) == 0) {
		if (!pthread->cancel_pending) {
			pthread->cancel_pending = 1;
			if (pthread->state != PS_DEAD)
				_thr_send_sig(pthread, SIGCANCEL);
		}
		THR_THREAD_UNLOCK(curthread, pthread);
	}
	return (ret);
}

int
_pthread_setcancelstate(int state, int *oldstate)
{
	struct pthread *curthread = _get_curthread();
	int oldval;

	oldval = curthread->cancel_enable;
	switch (state) {
	case PTHREAD_CANCEL_DISABLE:
		curthread->cancel_enable = 0;
		break;
	case PTHREAD_CANCEL_ENABLE:
		curthread->cancel_enable = 1;
		testcancel(curthread);
		break;
	default:
		return (EINVAL);
	}

	if (oldstate) {
		*oldstate = oldval ? PTHREAD_CANCEL_ENABLE :
			PTHREAD_CANCEL_DISABLE;
	}
	return (0);
}

int
_pthread_setcanceltype(int type, int *oldtype)
{
	struct pthread	*curthread = _get_curthread();
	int oldval;

	oldval = curthread->cancel_async;
	switch (type) {
	case PTHREAD_CANCEL_ASYNCHRONOUS:
		curthread->cancel_async = 1;
		testcancel(curthread);
		break;
	case PTHREAD_CANCEL_DEFERRED:
		curthread->cancel_async = 0;
		break;
	default:
		return (EINVAL);
	}

	if (oldtype) {
		*oldtype = oldval ? PTHREAD_CANCEL_ASYNCHRONOUS :
		 	PTHREAD_CANCEL_DEFERRED;
	}
	return (0);
}

void
_pthread_testcancel(void)
{
	struct pthread *curthread = _get_curthread();

	testcancel(curthread);
}

void
_thr_cancel_enter(struct pthread *curthread)
{
	curthread->cancel_point = 1;
	testcancel(curthread);
}

void
_thr_cancel_enter2(struct pthread *curthread, int maycancel)
{
	curthread->cancel_point = 1;
	if (__predict_false(SHOULD_CANCEL(curthread) &&
	    !THR_IN_CRITICAL(curthread))) {
		if (!maycancel)
			thr_wake(curthread->tid);
		else
			_pthread_exit(PTHREAD_CANCELED);
	}
}

void
_thr_cancel_leave(struct pthread *curthread, int maycancel)
{
	curthread->cancel_point = 0;
	if (__predict_false(SHOULD_CANCEL(curthread) &&
	    !THR_IN_CRITICAL(curthread) && maycancel))
		_pthread_exit(PTHREAD_CANCELED);
}

void
_pthread_cancel_enter(int maycancel)
{
	_thr_cancel_enter2(_get_curthread(), maycancel);
}

void
_pthread_cancel_leave(int maycancel)
{
	_thr_cancel_leave(_get_curthread(), maycancel);
}