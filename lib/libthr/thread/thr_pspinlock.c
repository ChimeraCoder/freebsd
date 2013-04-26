
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

#define SPIN_COUNT 100000

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
		_thr_umutex_init(&lck->s_lock);
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
	struct pthread *curthread = _get_curthread();
	struct pthread_spinlock	*lck;
	int ret;

	if (lock == NULL || (lck = *lock) == NULL)
		ret = EINVAL;
	else
		ret = THR_UMUTEX_TRYLOCK(curthread, &lck->s_lock);
	return (ret);
}

int
_pthread_spin_lock(pthread_spinlock_t *lock)
{
	struct pthread *curthread = _get_curthread();
	struct pthread_spinlock	*lck;
	int ret, count;

	if (lock == NULL || (lck = *lock) == NULL)
		ret = EINVAL;
	else {
		count = SPIN_COUNT;
		while ((ret = THR_UMUTEX_TRYLOCK(curthread, &lck->s_lock)) != 0) {
			while (lck->s_lock.m_owner) {
				if (!_thr_is_smp) {
					_pthread_yield();
				} else {
					CPU_SPINWAIT;

					if (--count <= 0) {
						count = SPIN_COUNT;
						_pthread_yield();
					}
				}
			}
		}
		ret = 0;
	}

	return (ret);
}

int
_pthread_spin_unlock(pthread_spinlock_t *lock)
{
	struct pthread *curthread = _get_curthread();
	struct pthread_spinlock	*lck;
	int ret;

	if (lock == NULL || (lck = *lock) == NULL)
		ret = EINVAL;
	else {
		ret = THR_UMUTEX_UNLOCK(curthread, &lck->s_lock);
	}
	return (ret);
}