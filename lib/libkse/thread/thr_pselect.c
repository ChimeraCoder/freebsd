
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "un-namespace.h"

#include "thr_private.h"

extern int	__pselect(int count, fd_set *rfds, fd_set *wfds, fd_set *efds, 
		    const struct timespec *timo, const sigset_t *mask);

int 		_pselect(int count, fd_set *rfds, fd_set *wfds, fd_set *efds, 
		    const struct timespec *timo, const sigset_t *mask);

__weak_reference(_pselect, pselect);

int 
_pselect(int count, fd_set *rfds, fd_set *wfds, fd_set *efds, 
	const struct timespec *timo, const sigset_t *mask)
{
	struct pthread *curthread = _get_curthread();
	int ret;

	_thr_cancel_enter(curthread);
	ret = __pselect(count, rfds, wfds, efds, timo, mask);
	_thr_cancel_leave(curthread, 1);

	return (ret);
}