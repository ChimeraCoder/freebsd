
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
#include <sys/param.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"

#include "thr_private.h"

__weak_reference(_pthread_setschedparam, pthread_setschedparam);

/*
 * Set a thread's scheduling parameters, this should be done
 * in kernel, doing it in userland is no-op.
 */
int
_pthread_setschedparam(pthread_t pthread, int policy, 
	const struct sched_param *param)
{
	struct pthread	*curthread = _get_curthread();
	int	ret;

	if (pthread == curthread)
		THR_LOCK(curthread);
	else if ((ret = _thr_find_thread(curthread, pthread,
		 /*include dead*/0)) != 0)
		return (ret);
	if (pthread->attr.sched_policy == policy &&
	    (policy == SCHED_OTHER ||
	     pthread->attr.prio == param->sched_priority)) {
		pthread->attr.prio = param->sched_priority;
		THR_THREAD_UNLOCK(curthread, pthread);
		return (0);
	}
	ret = _thr_setscheduler(pthread->tid, policy, param);
	if (ret == -1)
		ret = errno;
	else {
		pthread->attr.sched_policy = policy;
		pthread->attr.prio = param->sched_priority;
	}
	THR_THREAD_UNLOCK(curthread, pthread);
	return (ret);
}