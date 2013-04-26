
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
#include <pthread_np.h>
#include <sys/param.h>
#include <sys/cpuset.h>
#include "un-namespace.h"

#include "thr_private.h"

__weak_reference(_pthread_getaffinity_np, pthread_getaffinity_np);
__weak_reference(_pthread_setaffinity_np, pthread_setaffinity_np);

int
_pthread_setaffinity_np(pthread_t td, size_t cpusetsize, const cpuset_t *cpusetp)
{
	struct pthread	*curthread = _get_curthread();
	lwpid_t		tid;
	int		error;

	if (td == curthread) {
		error = cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID,
			-1, cpusetsize, cpusetp);
		if (error == -1)
			error = errno;
	} else if ((error = _thr_find_thread(curthread, td, 0)) == 0) {
		tid = TID(td);
		error = cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, tid,
			cpusetsize, cpusetp);
		if (error == -1)
			error = errno;
		THR_THREAD_UNLOCK(curthread, td);
	}
	return (error);
}

int
_pthread_getaffinity_np(pthread_t td, size_t cpusetsize, cpuset_t *cpusetp)
{
	struct pthread	*curthread = _get_curthread();
	lwpid_t tid;
	int error;

	if (td == curthread) {
		error = cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID,
			-1, cpusetsize, cpusetp);
		if (error == -1)
			error = errno;
	} else if ((error = _thr_find_thread(curthread, td, 0)) == 0) {
		tid = TID(td);
		error = cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, tid,
			    cpusetsize, cpusetp);
		if (error == -1)
			error = errno;
		THR_THREAD_UNLOCK(curthread, td);
	}
	return (error);
}