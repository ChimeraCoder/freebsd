
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
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int _pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind);
int _pthread_mutexattr_getkind_np(pthread_mutexattr_t attr);

__weak_reference(_pthread_mutexattr_setkind_np, pthread_mutexattr_setkind_np);
__weak_reference(_pthread_mutexattr_getkind_np, pthread_mutexattr_getkind_np);
__weak_reference(_pthread_mutexattr_gettype, pthread_mutexattr_gettype);
__weak_reference(_pthread_mutexattr_settype, pthread_mutexattr_settype);

int
_pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind)
{
	int	ret;
	if (attr == NULL || *attr == NULL) {
		errno = EINVAL;
		ret = -1;
	} else {
		(*attr)->m_type = kind;
		ret = 0;
	}
	return(ret);
}

int
_pthread_mutexattr_getkind_np(pthread_mutexattr_t attr)
{
	int	ret;
	if (attr == NULL) {
		errno = EINVAL;
		ret = -1;
	} else {
		ret = attr->m_type;
	}
	return(ret);
}

int
_pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	int	ret;
	if (attr == NULL || *attr == NULL || type >= PTHREAD_MUTEX_TYPE_MAX) {
		errno = EINVAL;
		ret = -1;
	} else {
		(*attr)->m_type = type;
		ret = 0;
	}
	return(ret);
}

int
_pthread_mutexattr_gettype(pthread_mutexattr_t *attr, int *type)
{
	int	ret;

	if (attr == NULL || *attr == NULL || (*attr)->m_type >=
	    PTHREAD_MUTEX_TYPE_MAX) {
		ret = EINVAL;
	} else {
		*type = (*attr)->m_type;
		ret = 0;
	}
	return ret;
}