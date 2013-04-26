
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
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int	__open(const char *path, int flags,...);

__weak_reference(__open, open);

int
__open(const char *path, int flags,...)
{
	struct pthread *curthread = _get_curthread();
	int	ret;
	int	mode = 0;
	va_list	ap;

	_thr_cancel_enter(curthread);
	
	/* Check if the file is being created: */
	if (flags & O_CREAT) {
		/* Get the creation mode: */
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	}
	
	ret = __sys_open(path, flags, mode);
	/*
	 * To avoid possible file handle leak, 
	 * only check cancellation point if it is failure
	 */
	_thr_cancel_leave(curthread, (ret == -1));

	return ret;
}