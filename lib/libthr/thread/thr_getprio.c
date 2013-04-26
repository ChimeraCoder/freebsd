
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

__weak_reference(_pthread_getprio, pthread_getprio);

int
_pthread_getprio(pthread_t pthread)
{
	int policy, ret;
	struct sched_param param;

	if ((ret = _pthread_getschedparam(pthread, &policy, &param)) == 0)
		ret = param.sched_priority;
	else {
		/* Invalid thread: */
		errno = ret;
		ret = -1;
	}

	/* Return the thread priority or an error status: */
	return (ret);
}