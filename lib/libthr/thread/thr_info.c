
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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <pthread_np.h>
#include "un-namespace.h"

#include "thr_private.h"

__weak_reference(_pthread_set_name_np, pthread_set_name_np);

/* Set the thread name for debug. */
void
_pthread_set_name_np(pthread_t thread, const char *name)
{
	struct pthread *curthread = _get_curthread();
	int ret = 0;

	if (curthread == thread) {
		if (thr_set_name(thread->tid, name))
			ret = errno;
	} else {
		if ((ret=_thr_find_thread(curthread, thread, 0)) == 0) {
			if (thread->state != PS_DEAD) {
				if (thr_set_name(thread->tid, name))
					ret = errno;
			}
			THR_THREAD_UNLOCK(curthread, thread);
		}
	}
#if 0
	/* XXX should return error code. */
	return (ret);
#endif
}