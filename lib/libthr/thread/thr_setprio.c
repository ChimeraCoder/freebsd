
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

__weak_reference(_pthread_setprio, pthread_setprio);

int
_pthread_setprio(pthread_t pthread, int prio)
{
	struct pthread	*curthread = _get_curthread();
	struct sched_param	param;
	int	ret;

	param.sched_priority = prio;
	if (pthread == curthread)
		THR_LOCK(curthread);
	else if ((ret = _thr_find_thread(curthread, pthread, /*include dead*/0)))
		return (ret);
	if (pthread->attr.sched_policy == SCHED_OTHER ||
	    pthread->attr.prio == prio) {
		pthread->attr.prio = prio;
		ret = 0;
	} else {
		ret = _thr_setscheduler(pthread->tid,
			pthread->attr.sched_policy, &param);
		if (ret == -1)
			ret = errno;
		else
			pthread->attr.prio = prio;
	}
	THR_THREAD_UNLOCK(curthread, pthread);
	return (ret);
}