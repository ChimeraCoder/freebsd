
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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_mutexattr_getprioceiling, pthread_mutexattr_getprioceiling);
__weak_reference(_pthread_mutexattr_setprioceiling, pthread_mutexattr_setprioceiling);
__weak_reference(_pthread_mutex_getprioceiling, pthread_mutex_getprioceiling);
__weak_reference(_pthread_mutex_setprioceiling, pthread_mutex_setprioceiling);

int
_pthread_mutexattr_getprioceiling(pthread_mutexattr_t *mattr, int *prioceiling)
{
	int ret = 0;

	if ((mattr == NULL) || (*mattr == NULL))
		ret = EINVAL;
	else if ((*mattr)->m_protocol != PTHREAD_PRIO_PROTECT)
		ret = EINVAL;
	else
		*prioceiling = (*mattr)->m_ceiling;

	return(ret);
}

int
_pthread_mutexattr_setprioceiling(pthread_mutexattr_t *mattr, int prioceiling)
{
	int ret = 0;

	if ((mattr == NULL) || (*mattr == NULL))
		ret = EINVAL;
	else if ((*mattr)->m_protocol != PTHREAD_PRIO_PROTECT)
		ret = EINVAL;
	else
		(*mattr)->m_ceiling = prioceiling;

	return(ret);
}

int
_pthread_mutex_getprioceiling(pthread_mutex_t *mutex,
			      int *prioceiling)
{
	int ret;

	if ((mutex == NULL) || (*mutex == NULL))
		ret = EINVAL;
	else if ((*mutex)->m_protocol != PTHREAD_PRIO_PROTECT)
		ret = EINVAL;
	else {
		*prioceiling = (*mutex)->m_prio;
		ret = 0;
	}
	return (ret);
}

int
_pthread_mutex_setprioceiling(pthread_mutex_t *mutex,
			      int prioceiling, int *old_ceiling)
{
	int ret = 0;
	int tmp;

	if ((mutex == NULL) || (*mutex == NULL))
		ret = EINVAL;
	else if ((*mutex)->m_protocol != PTHREAD_PRIO_PROTECT)
		ret = EINVAL;
	/* Lock the mutex: */
	else if ((ret = _pthread_mutex_lock(mutex)) == 0) {
		tmp = (*mutex)->m_prio;
		/* Set the new ceiling: */
		(*mutex)->m_prio = prioceiling;

		/* Unlock the mutex: */
		ret = _pthread_mutex_unlock(mutex);

		/* Return the old ceiling: */
		*old_ceiling = tmp;
	}
	return(ret);
}