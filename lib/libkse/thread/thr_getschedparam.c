
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

__weak_reference(_pthread_getschedparam, pthread_getschedparam);

int
_pthread_getschedparam(pthread_t pthread, int *policy, 
	struct sched_param *param)
{
	struct pthread *curthread = _get_curthread();
	int ret, tmp;

	if ((param == NULL) || (policy == NULL))
		/* Return an invalid argument error: */
		ret = EINVAL;
	else if (pthread == curthread) {
		/*
		 * Avoid searching the thread list when it is the current
		 * thread.
		 */
		THR_SCHED_LOCK(curthread, curthread);
		param->sched_priority =
		    THR_BASE_PRIORITY(pthread->base_priority);
		tmp = pthread->attr.sched_policy;
		THR_SCHED_UNLOCK(curthread, curthread);
		*policy = tmp;
		ret = 0;
	}
	/* Find the thread in the list of active threads. */
	else if ((ret = _thr_ref_add(curthread, pthread, /*include dead*/0))
	    == 0) {
		THR_SCHED_LOCK(curthread, pthread);
		param->sched_priority =
		    THR_BASE_PRIORITY(pthread->base_priority);
		tmp = pthread->attr.sched_policy;
		THR_SCHED_UNLOCK(curthread, pthread);
		_thr_ref_delete(curthread, pthread);
		*policy = tmp;
	}
	return (ret);
}