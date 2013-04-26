
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

/*
 * Find a thread in the linked list of active threads and add a reference
 * to it.  Threads with positive reference counts will not be deallocated
 * until all references are released.
 */
int
_thr_ref_add(struct pthread *curthread, struct pthread *thread,
    int include_dead)
{
	kse_critical_t crit;
	struct pthread *pthread;
	struct kse *curkse;

	if (thread == NULL)
		/* Invalid thread: */
		return (EINVAL);

	crit = _kse_critical_enter();
	curkse = _get_curkse();
	KSE_LOCK_ACQUIRE(curkse, &_thread_list_lock);
	pthread = _thr_hash_find(thread);
	if (pthread) {
		if ((include_dead == 0) &&
		    ((pthread->state == PS_DEAD) ||
		    ((pthread->state == PS_DEADLOCK) ||
		    ((pthread->flags & THR_FLAGS_EXITING) != 0))))
			pthread = NULL;
		else {
			pthread->refcount++;
			if (curthread != NULL)
				curthread->critical_count++;
		}
	}
	KSE_LOCK_RELEASE(curkse, &_thread_list_lock);
	_kse_critical_leave(crit);

	/* Return zero if the thread exists: */
	return ((pthread != NULL) ? 0 : ESRCH);
}

void
_thr_ref_delete(struct pthread *curthread, struct pthread *thread)
{
	kse_critical_t crit;
	struct kse *curkse;

	if (thread != NULL) {
		crit = _kse_critical_enter();
		curkse = _get_curkse();
		KSE_LOCK_ACQUIRE(curkse, &_thread_list_lock);
		thread->refcount--;
		if (curthread != NULL)
			curthread->critical_count--;
		if ((thread->refcount == 0) &&
		    (thread->tlflags & TLFLAGS_GC_SAFE) != 0)
			THR_GCLIST_ADD(thread);
		KSE_LOCK_RELEASE(curkse, &_thread_list_lock);
		_kse_critical_leave(crit);
	}
}