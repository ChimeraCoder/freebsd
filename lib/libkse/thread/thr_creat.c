
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
#include <fcntl.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

extern int __creat(const char *, mode_t);

int ___creat(const char *path, mode_t mode);

__weak_reference(___creat, creat);

int
___creat(const char *path, mode_t mode)
{
	struct pthread *curthread = _get_curthread();
	int ret;

	_thr_cancel_enter(curthread);
	ret = __creat(path, mode);
	/*
	 * To avoid possible file handle leak, 
	 * only check cancellation point if it is failure
	 */
	_thr_cancel_leave(curthread, (ret == -1));
	
	return ret;
}