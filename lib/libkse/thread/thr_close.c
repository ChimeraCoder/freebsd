
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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int __close(int fd);

__weak_reference(__close, close);

int
__close(int fd)
{
	struct pthread	*curthread = _get_curthread();
	int	ret;

	_thr_cancel_enter(curthread);
	ret = __sys_close(fd);
	_thr_cancel_leave(curthread, 1);
	
	return (ret);
}