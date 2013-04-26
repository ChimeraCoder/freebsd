
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
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_barrierattr_destroy, pthread_barrierattr_destroy);
__weak_reference(_pthread_barrierattr_init, pthread_barrierattr_init);
__weak_reference(_pthread_barrierattr_setpshared,
	pthread_barrierattr_setpshared);
__weak_reference(_pthread_barrierattr_getpshared,
	pthread_barrierattr_getpshared);

int
_pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{

	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	free(*attr);
	return (0);
}

int
_pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr,
	int *pshared)
{

	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	*pshared = (*attr)->pshared;
	return (0);
}

int
_pthread_barrierattr_init(pthread_barrierattr_t *attr)
{

	if (attr == NULL)
		return (EINVAL);

	if ((*attr = malloc(sizeof(struct pthread_barrierattr))) == NULL)
		return (ENOMEM);

	(*attr)->pshared = PTHREAD_PROCESS_PRIVATE;
	return (0);
}

int
_pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{

	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	/* Only PTHREAD_PROCESS_PRIVATE is supported. */
	if (pshared != PTHREAD_PROCESS_PRIVATE)
		return (EINVAL);

	(*attr)->pshared = pshared;
	return (0);
}