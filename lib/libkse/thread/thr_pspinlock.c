
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
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include "un-namespace.h"

#include "atomic_ops.h"
#include "thr_private.h"

#define SPIN_COUNT 10000

__weak_reference(_pthread_spin_init, pthread_spin_init);
__weak_reference(_pthread_spin_destroy, pthread_spin_destroy);
__weak_reference(_pthread_spin_trylock, pthread_spin_trylock);
__weak_reference(_pthread_spin_lock, pthread_spin_lock);
__weak_reference(_pthread_spin_unlock, pthread_spin_unlock);

int
_pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
	struct pthread_spinlock	*lck;
	int ret;

	if (lock == NULL || pshared != PTHREAD_PROCESS_PRIVATE)
		ret = EINVAL;
	else if ((lck = malloc(sizeof(struct pthread_spinlock))) == NULL)
		ret = ENOMEM;
	else {
		lck->s_lock = 0;
		lck->s_owner= NULL;
		*lock = lck;
		ret = 0;
	}

	return (ret);
}

int
_pthread_spin_destroy(pthread_spinlock_t *lock)
{
	int ret;

	if (lock == NULL || *lock == NULL)
		ret = EINVAL;
	else if ((*lock)->s_owner != NULL)
		ret = EBUSY;
	else {
		free(*lock);
		*lock = NULL;
		ret = 0;
	}

	return (ret);
}

int
_pthread_spin_trylock(pthread_spinlock_t *lock)
{
	struct pthread_spinlock	*lck;
	struct pthread *self = _pthread_self();
	int oldval, ret;

	if (lock == NULL || (lck = *lock) == NULL)
		ret = EINVAL;
	else if (lck->s_owner == self)
		ret = EDEADLK;
	else if (lck->s_lock != 0)
		ret = EBUSY;
	else {
		atomic_swap_int(&(lck)->s_lock, 1, &oldval);
		if (oldval)
			ret = EBUSY;
		else {
			lck->s_owner = _pthread_self();
			ret = 0;
		}
	}
	return (ret);
}

int
_pthread_spin_lock(pthread_spinlock_t *lock)
{
	struct pthread_spinlock	*lck;
	struct pthread *self = _pthread_self();
	int count, oldval, ret;

	if (lock == NULL || (lck = *lock) == NULL)
		ret = EINVAL;
	else if (lck->s_owner == self)
		ret = EDEADLK;
	else {
		do {
			count = SPIN_COUNT;
			while (lck->s_lock) {
#ifdef __i386__
				/* tell cpu we are spinning */
				__asm __volatile("pause");
#endif
				if (--count <= 0) {
					count = SPIN_COUNT;
					_pthread_yield();
				}
			}
			atomic_swap_int(&(lck)->s_lock, 1, &oldval);
		} while (oldval);

		lck->s_owner = self;
		ret = 0;
	}

	return (ret);
}

int
_pthread_spin_unlock(pthread_spinlock_t *lock)
{
	struct pthread_spinlock	*lck;
	int ret;

	if (lock == NULL || (lck = *lock) == NULL)
		ret = EINVAL;
	else {
		if (lck->s_owner != _pthread_self())
			ret = EPERM;
		else {
			lck->s_owner = NULL;
			atomic_swap_int(&lck->s_lock, 0, &ret);
			ret = 0;
		}
	}

	return (ret);
}