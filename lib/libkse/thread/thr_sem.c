
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
#include <sys/queue.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <_semaphore.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "thr_private.h"

__weak_reference(_sem_init, sem_init);
__weak_reference(_sem_wait, sem_wait);
__weak_reference(_sem_timedwait, sem_timedwait);
__weak_reference(_sem_post, sem_post);


static inline int
sem_check_validity(sem_t *sem)
{

	if ((sem != NULL) && ((*sem)->magic == SEM_MAGIC))
		return (0);
	else {
		errno = EINVAL;
		return (-1);
	}
}

static void
decrease_nwaiters(void *arg)
{
	sem_t *sem = (sem_t *)arg;

	(*sem)->nwaiters--;
	/*
	 * this function is called from cancellation point,
	 * the mutex should already be hold.
	 */
	_pthread_mutex_unlock(&(*sem)->lock);
}

static sem_t
sem_alloc(unsigned int value, semid_t semid, int system_sem)
{
	sem_t sem;

	if (value > SEM_VALUE_MAX) {
		errno = EINVAL;
		return (NULL);
	}

	sem = (sem_t)malloc(sizeof(struct sem));
	if (sem == NULL) {
		errno = ENOSPC;
		return (NULL);
	}

	/*
	 * Initialize the semaphore.
	 */
	if (_pthread_mutex_init(&sem->lock, NULL) != 0) {
		free(sem);
		errno = ENOSPC;
		return (NULL);
	}

	if (_pthread_cond_init(&sem->gtzero, NULL) != 0) {
		_pthread_mutex_destroy(&sem->lock);
		free(sem);
		errno = ENOSPC;
		return (NULL);
	}

	sem->count = (u_int32_t)value;
	sem->nwaiters = 0;
	sem->magic = SEM_MAGIC;
	sem->semid = semid;
	sem->syssem = system_sem;
	return (sem);
}

int
_sem_init(sem_t *sem, int pshared, unsigned int value)
{
	semid_t semid;

	semid = (semid_t)SEM_USER;
	if ((pshared != 0) && (ksem_init(&semid, value) != 0))
		return (-1);

	(*sem) = sem_alloc(value, semid, pshared);
	if ((*sem) == NULL) {
		if (pshared != 0)
			ksem_destroy(semid);
		return (-1);
	}
	return (0);
}

int
_sem_wait(sem_t *sem)
{
	struct pthread *curthread;
	int retval;

	if (sem_check_validity(sem) != 0)
		return (-1);

	curthread = _get_curthread();
	if ((*sem)->syssem != 0) {
		_thr_cancel_enter(curthread);
		retval = ksem_wait((*sem)->semid);
		_thr_cancel_leave(curthread, retval != 0);
	}
	else {
		_pthread_testcancel();
		_pthread_mutex_lock(&(*sem)->lock);

		while ((*sem)->count <= 0) {
			(*sem)->nwaiters++;
			THR_CLEANUP_PUSH(curthread, decrease_nwaiters, sem);
			_pthread_cond_wait(&(*sem)->gtzero, &(*sem)->lock);
			THR_CLEANUP_POP(curthread, 0);
			(*sem)->nwaiters--;
		}
		(*sem)->count--;

		_pthread_mutex_unlock(&(*sem)->lock);

		retval = 0;
	}
	return (retval);
}

int
_sem_timedwait(sem_t * __restrict sem,
    const struct timespec * __restrict abs_timeout)
{
	struct pthread *curthread;
	int retval;
	int timeout_invalid;

	if (sem_check_validity(sem) != 0)
		return (-1);

	if ((*sem)->syssem != 0) {
		curthread = _get_curthread();
		_thr_cancel_enter(curthread);
		retval = ksem_timedwait((*sem)->semid, abs_timeout);
		_thr_cancel_leave(curthread, retval != 0);
	}
	else {
		/*
		 * The timeout argument is only supposed to
		 * be checked if the thread would have blocked.
		 * This is checked outside of the lock so a
		 * segfault on an invalid address doesn't end
		 * up leaving the mutex locked.
		 */
		_pthread_testcancel();
		timeout_invalid = (abs_timeout->tv_nsec >= 1000000000) ||
		    (abs_timeout->tv_nsec < 0);
		_pthread_mutex_lock(&(*sem)->lock);

		if ((*sem)->count <= 0) {
			if (timeout_invalid) {
				_pthread_mutex_unlock(&(*sem)->lock);
				errno = EINVAL;
				return (-1);
			}
			(*sem)->nwaiters++;
			_pthread_cleanup_push(decrease_nwaiters, sem);
			_pthread_cond_timedwait(&(*sem)->gtzero,
			    &(*sem)->lock, abs_timeout);
			_pthread_cleanup_pop(0);
			(*sem)->nwaiters--;
		}
		if ((*sem)->count == 0) {
			errno = ETIMEDOUT;
			retval = -1;
		}
		else {
			(*sem)->count--;
			retval = 0;
		}	

		_pthread_mutex_unlock(&(*sem)->lock);
	}

	return (retval);
}

int
_sem_post(sem_t *sem)
{
	struct pthread *curthread;
	int retval;
	
	if (sem_check_validity(sem) != 0)
		return (-1);

	if ((*sem)->syssem != 0)
		retval = ksem_post((*sem)->semid);
	else {
		/*
		 * sem_post() is required to be safe to call from within
		 * signal handlers.  Thus, we must enter a critical region.
		 */
		curthread = _get_curthread();
		_thr_critical_enter(curthread);
		_pthread_mutex_lock(&(*sem)->lock);

		(*sem)->count++;
		if ((*sem)->nwaiters > 0)
			_pthread_cond_signal(&(*sem)->gtzero);

		_pthread_mutex_unlock(&(*sem)->lock);
		_thr_critical_leave(curthread);
		retval = 0;
	}

	return (retval);
}