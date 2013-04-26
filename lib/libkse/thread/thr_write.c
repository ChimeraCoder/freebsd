
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
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "thr_private.h"

__ssize_t __write(int fd, const void *buf, size_t nbytes);

__weak_reference(__write, write);

ssize_t
__write(int fd, const void *buf, size_t nbytes)
{
	struct pthread *curthread = _get_curthread();
	ssize_t	ret;

	_thr_cancel_enter(curthread);
	ret = __sys_write(fd, buf, nbytes);
	_thr_cancel_leave(curthread, 1);

	return ret;
}