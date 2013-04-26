
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
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "un-namespace.h"

#include "thr_private.h"

#undef pthread_cleanup_push
#undef pthread_cleanup_pop

/* old binary compatible interfaces */
__weak_reference(_pthread_cleanup_push, pthread_cleanup_push);
__weak_reference(_pthread_cleanup_pop, pthread_cleanup_pop);

void
__pthread_cleanup_push_imp(void (*routine)(void *), void *arg,
	struct _pthread_cleanup_info *info)
{
	struct pthread	*curthread = _get_curthread();
	struct pthread_cleanup *newbuf;

	newbuf = (void *)info;
	newbuf->routine = routine;
	newbuf->routine_arg = arg;
	newbuf->onheap = 0;
	newbuf->prev = curthread->cleanup;
	curthread->cleanup = newbuf;
}

void
__pthread_cleanup_pop_imp(int execute)
{
	struct pthread	*curthread = _get_curthread();
	struct pthread_cleanup *old;

	if ((old = curthread->cleanup) != NULL) {
		curthread->cleanup = old->prev;
		if (execute)
			old->routine(old->routine_arg);
		if (old->onheap)
			free(old);
	}
}

void
_pthread_cleanup_push(void (*routine) (void *), void *arg)
{
	struct pthread	*curthread = _get_curthread();
	struct pthread_cleanup *newbuf;
#ifdef _PTHREAD_FORCED_UNWIND
	curthread->unwind_disabled = 1;
#endif
	if ((newbuf = (struct pthread_cleanup *)
	    malloc(sizeof(struct _pthread_cleanup_info))) != NULL) {
		newbuf->routine = routine;
		newbuf->routine_arg = arg;
		newbuf->onheap = 1;
		newbuf->prev = curthread->cleanup;
		curthread->cleanup = newbuf;
	}
}

void
_pthread_cleanup_pop(int execute)
{
	__pthread_cleanup_pop_imp(execute);
}