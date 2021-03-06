
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
#include <errno.h>
#include <pthread.h>
#include "libc_private.h"
#include "thr_private.h"

#undef errno
extern	int	errno;

int *
__error(void)
{
	struct pthread *curthread;

	if (__isthreaded == 0)
		return (&errno);
	else if (_kse_in_critical())
		return &(_get_curkse()->k_error);
	else {
		curthread = _get_curthread();
		if ((curthread == NULL) || (curthread == _thr_initial))
			return (&errno);
		else
			return (&curthread->error);
	}
}