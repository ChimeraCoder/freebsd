
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
#define ONCE_WAIT		0x03

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

	if (atomic_cmpset_rel_int(&once_control->state, ONCE_IN_PROGRESS, ONCE_NEVER_DONE))
		return;
	atomic_store_rel_int(&once_control->state, ONCE_NEVER_DONE);
	_thr_umtx_wake(&once_control->state, INT_MAX, 0);
}

int
_pthread_once(pthread_once_t *once_control, void (*init_routine) (void))
{
	struct pthread *curthread;
	int state;

	_thr_check_init();

	for (;;) {
		state = once_control->state;
		if (state == ONCE_DONE)
			return (0);
		if (state == ONCE_NEVER_DONE) {
			if (atomic_cmpset_acq_int(&once_control->state, state, ONCE_IN_PROGRESS))
				break;
		} else if (state == ONCE_IN_PROGRESS) {
			if (atomic_cmpset_acq_int(&once_control->state, state, ONCE_WAIT))
				_thr_umtx_wait_uint(&once_control->state, ONCE_WAIT, NULL, 0);
		} else if (state == ONCE_WAIT) {
			_thr_umtx_wait_uint(&once_control->state, state, NULL, 0);
		} else
			return (EINVAL);
        }

	curthread = _get_curthread();
	THR_CLEANUP_PUSH(curthread, once_cancel_handler, once_control);
	init_routine();
	THR_CLEANUP_POP(curthread, 0);
	if (atomic_cmpset_rel_int(&once_control->state, ONCE_IN_PROGRESS, ONCE_DONE))
		return (0);
	atomic_store_rel_int(&once_control->state, ONCE_DONE);
	_thr_umtx_wake(&once_control->state, INT_MAX, 0);
	return (0);
}

void
_thr_once_init()
{
}