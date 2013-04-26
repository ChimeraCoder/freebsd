
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
#include <pthread.h>
#include "thr_private.h"

int _sched_yield(void);

__weak_reference(_sched_yield, sched_yield);
__weak_reference(_pthread_yield, pthread_yield);

int
_sched_yield(void)
{
	struct pthread	*curthread = _get_curthread();

	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM)
		return (__sys_sched_yield());

	/* Reset the accumulated time slice value for the current thread: */
	curthread->slice_usec = -1;

	/* Schedule the next thread: */
	_thr_sched_switch(curthread);
	/* Always return no error. */
	return(0);
}

/* Draft 4 yield */
void
_pthread_yield(void)
{
	struct pthread	*curthread = _get_curthread();

	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM) {
		__sys_sched_yield();
		return;
	}

	/* Reset the accumulated time slice value for the current thread: */
	curthread->slice_usec = -1;

	/* Schedule the next thread: */
	_thr_sched_switch(curthread);
}