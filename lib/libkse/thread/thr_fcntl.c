
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
#include <stdarg.h>
#include <fcntl.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int __fcntl(int fd, int cmd,...);
extern int __fcntl_compat(int fd, int cmd,...);

__weak_reference(__fcntl, fcntl);

int
__fcntl(int fd, int cmd,...)
{
	struct pthread *curthread = _get_curthread();
	int	ret, check = 1;
	va_list	ap;
	
	_thr_cancel_enter(curthread);

	va_start(ap, cmd);
	switch (cmd) {
	case F_DUPFD:
		ret = __sys_fcntl(fd, cmd, va_arg(ap, int));
		/*
		 * To avoid possible file handle leak, 
		 * only check cancellation point if it is failure
		 */
		check = (ret == -1);
		break;
	case F_SETFD:
	case F_SETFL:
		ret = __sys_fcntl(fd, cmd, va_arg(ap, int));
		break;
	case F_GETFD:
	case F_GETFL:
		ret = __sys_fcntl(fd, cmd);
		break;
	default:
		ret = __fcntl_compat(fd, cmd, va_arg(ap, void *));
	}
	va_end(ap);

	_thr_cancel_leave(curthread, check);

	return (ret);
}