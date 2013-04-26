
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
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int	__select(int numfds, fd_set *readfds, fd_set *writefds,
	    fd_set *exceptfds, struct timeval *timeout);

__weak_reference(__select, select);

int 
__select(int numfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	struct timeval *timeout)
{
	struct pthread *curthread = _get_curthread();
	struct timespec ts;
	int ret;

	if (numfds == 0 && timeout != NULL) {
		TIMEVAL_TO_TIMESPEC(timeout, &ts);
		ret = _nanosleep(&ts, NULL);
	} else {
		_thr_cancel_enter(curthread);
		ret = __sys_select(numfds, readfds, writefds, exceptfds, timeout);
		_thr_cancel_leave(curthread, 1);
	}
	return (ret);
}