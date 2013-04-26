
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

/*
 * This function needs to reference the global error variable which is
 * normally hidden from the user. 
 */
#ifdef errno
#undef errno;
#endif
extern int      errno;

void	_thread_seterrno(pthread_t thread, int error);

void
_thread_seterrno(pthread_t thread, int error)
{
	/* Check for the initial thread: */
	if (thread == _thr_initial)
		/* The initial thread always uses the global error variable: */
		errno = error;
	else
		/*
		 * Threads other than the initial thread always use the error
		 * field in the thread structureL 
		 */
		thread->error = error;
}