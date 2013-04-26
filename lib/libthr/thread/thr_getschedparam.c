
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
#include <sys/types.h>
#include <sys/rtprio.h>
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
	int ret = 0;

	if (policy == NULL || param == NULL)
		return (EINVAL);

	/*
	 * Avoid searching the thread list when it is the current
	 * thread.
	 */
	if (pthread == curthread)
		THR_LOCK(curthread);
	else if ((ret = _thr_find_thread(curthread, pthread, /*include dead*/0)))
		return (ret);
	*policy = pthread->attr.sched_policy;
	param->sched_priority = pthread->attr.prio;
	THR_THREAD_UNLOCK(curthread, pthread);
	return (ret);
}