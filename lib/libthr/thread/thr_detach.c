
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
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"

#include "thr_private.h"

__weak_reference(_pthread_detach, pthread_detach);

int
_pthread_detach(pthread_t pthread)
{
	struct pthread *curthread = _get_curthread();
	int rval;

	if (pthread == NULL)
		return (EINVAL);

	if ((rval = _thr_find_thread(curthread, pthread,
			/*include dead*/1)) != 0) {
		return (rval);
	}

	/* Check if the thread is already detached or has a joiner. */
	if ((pthread->flags & THR_FLAGS_DETACHED) != 0 ||
	    (pthread->joiner != NULL)) {
		THR_THREAD_UNLOCK(curthread, pthread);
		return (EINVAL);
	}

	/* Flag the thread as detached. */
	pthread->flags |= THR_FLAGS_DETACHED;
	_thr_try_gc(curthread, pthread); /* thread lock released */

	return (0);
}