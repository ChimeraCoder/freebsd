
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

__weak_reference(_pthread_once, pthread_once);

#define ONCE_NEVER_DONE		PTHREAD_NEEDS_INIT
#define ONCE_DONE		PTHREAD_DONE_INIT
#define	ONCE_IN_PROGRESS	0x02
#define	ONCE_MASK		0x03

static pthread_mutex_t		once_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t		once_cv = PTHREAD_COND_INITIALIZER;

/*
 * POSIX:
 * The pthread_once() function is not a cancellation point. However,
 * if init_routine is a cancellation point and is canceled, the effect
 * on once_control shall be as if pthread_once() was never called.
 */
 
static void
once_cancel_handler(void *arg)
{
	pthread_once_t *once_control = arg;

	_pthread_mutex_lock(&once_lock);
	once_control->state = ONCE_NEVER_DONE;
	_pthread_mutex_unlock(&once_lock);
	_pthread_cond_broadcast(&once_cv);
}

int
_pthread_once(pthread_once_t *once_control, void (*init_routine) (void))
{
	struct pthread *curthread;
	int wakeup = 0;

	if (once_control->state == ONCE_DONE)
		return (0);
	_pthread_mutex_lock(&once_lock);
	while (*(volatile int *)&(once_control->state) == ONCE_IN_PROGRESS)
		_pthread_cond_wait(&once_cv, &once_lock);
	/*
	 * If previous thread was canceled, then the state still
	 * could be ONCE_NEVER_DONE, we need to check it again.
	 */
	if (*(volatile int *)&(once_control->state) == ONCE_NEVER_DONE) {
		once_control->state = ONCE_IN_PROGRESS;
		_pthread_mutex_unlock(&once_lock);
		curthread = _get_curthread();
		THR_CLEANUP_PUSH(curthread, once_cancel_handler, once_control);
		init_routine();
		THR_CLEANUP_POP(curthread, 0);
		_pthread_mutex_lock(&once_lock);
		once_control->state = ONCE_DONE;
		wakeup = 1;
	}
	_pthread_mutex_unlock(&once_lock);
	if (wakeup)
		_pthread_cond_broadcast(&once_cv);
	return (0);
}