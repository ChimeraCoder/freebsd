
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"

#include "thr_private.h"

__weak_reference(_pthread_condattr_init, pthread_condattr_init);
__weak_reference(_pthread_condattr_destroy, pthread_condattr_destroy);
__weak_reference(_pthread_condattr_getclock, pthread_condattr_getclock);
__weak_reference(_pthread_condattr_setclock, pthread_condattr_setclock);
__weak_reference(_pthread_condattr_getpshared, pthread_condattr_getpshared);
__weak_reference(_pthread_condattr_setpshared, pthread_condattr_setpshared);

int
_pthread_condattr_init(pthread_condattr_t *attr)
{
	pthread_condattr_t pattr;
	int ret;

	if ((pattr = (pthread_condattr_t)
	    malloc(sizeof(struct pthread_cond_attr))) == NULL) {
		ret = ENOMEM;
	} else {
		memcpy(pattr, &_pthread_condattr_default,
		    sizeof(struct pthread_cond_attr));
		*attr = pattr;
		ret = 0;
	}
	return (ret);
}

int
_pthread_condattr_destroy(pthread_condattr_t *attr)
{
	int	ret;

	if (attr == NULL || *attr == NULL) {
		ret = EINVAL;
	} else {
		free(*attr);
		*attr = NULL;
		ret = 0;
	}
	return(ret);
}

int
_pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id)
{
	if (attr == NULL || *attr == NULL)
		return (EINVAL);
	*clock_id = (*attr)->c_clockid;
	return (0);
}

int
_pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id)
{
	if (attr == NULL || *attr == NULL)
		return (EINVAL);
	if (clock_id != CLOCK_REALTIME &&
	    clock_id != CLOCK_VIRTUAL &&
	    clock_id != CLOCK_PROF &&
	    clock_id != CLOCK_MONOTONIC) {
		return  (EINVAL);
	}
	(*attr)->c_clockid = clock_id;
	return (0);
}

int
_pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared)
{
	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	*pshared = PTHREAD_PROCESS_PRIVATE;
	return (0);
}

int
_pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	if  (pshared != PTHREAD_PROCESS_PRIVATE)
		return (EINVAL);
	return (0);
}