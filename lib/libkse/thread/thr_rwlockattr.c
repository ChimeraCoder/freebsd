
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

__weak_reference(_pthread_rwlockattr_destroy, pthread_rwlockattr_destroy);
__weak_reference(_pthread_rwlockattr_getpshared, pthread_rwlockattr_getpshared);
__weak_reference(_pthread_rwlockattr_init, pthread_rwlockattr_init);
__weak_reference(_pthread_rwlockattr_setpshared, pthread_rwlockattr_setpshared);

int
_pthread_rwlockattr_destroy(pthread_rwlockattr_t *rwlockattr)
{
	pthread_rwlockattr_t prwlockattr;

	if (rwlockattr == NULL)
		return(EINVAL);

	prwlockattr = *rwlockattr;

	if (prwlockattr == NULL)
		return(EINVAL);

	free(prwlockattr);

	return(0);
}

int
_pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *rwlockattr,
	int *pshared)
{
	*pshared = (*rwlockattr)->pshared;

	return(0);
}

int
_pthread_rwlockattr_init(pthread_rwlockattr_t *rwlockattr)
{
	pthread_rwlockattr_t prwlockattr;

	if (rwlockattr == NULL)
		return(EINVAL);

	prwlockattr = (pthread_rwlockattr_t)
		malloc(sizeof(struct pthread_rwlockattr));

	if (prwlockattr == NULL)
		return(ENOMEM);

	prwlockattr->pshared 	= PTHREAD_PROCESS_PRIVATE;
	*rwlockattr		= prwlockattr;

	return(0);
}

int
_pthread_rwlockattr_setpshared(pthread_rwlockattr_t *rwlockattr, int pshared)
{
	/* Only PTHREAD_PROCESS_PRIVATE is supported. */
	if (pshared != PTHREAD_PROCESS_PRIVATE)
		return(EINVAL);

	(*rwlockattr)->pshared = pshared;

	return(0);
}