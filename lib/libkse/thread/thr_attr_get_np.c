
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
#include <string.h>
#include <pthread.h>
#include <pthread_np.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_attr_get_np, pthread_attr_get_np);

int
_pthread_attr_get_np(pthread_t pid, pthread_attr_t *dst)
{
	struct pthread *curthread;
	struct pthread_attr attr;
	int	ret;

	if (pid == NULL || dst == NULL || *dst == NULL)
		return (EINVAL);

	curthread = _get_curthread();
	if ((ret = _thr_ref_add(curthread, pid, /*include dead*/0)) != 0)
		return (ret);
	attr = pid->attr;
	_thr_ref_delete(curthread, pid);
	memcpy(*dst, &attr, sizeof(struct pthread_attr));

	return (0);
}