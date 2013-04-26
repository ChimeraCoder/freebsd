
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
#include <stdlib.h>
#include <time.h>
#include <_semaphore.h>
#include "un-namespace.h"

#include "thr_private.h"

FB10_COMPAT(_sem_init_compat, sem_init);
FB10_COMPAT(_sem_destroy_compat, sem_destroy);
FB10_COMPAT(_sem_getvalue_compat, sem_getvalue);
FB10_COMPAT(_sem_trywait_compat, sem_trywait);
FB10_COMPAT(_sem_wait_compat, sem_wait);
FB10_COMPAT(_sem_timedwait_compat, sem_timedwait);
FB10_COMPAT(_sem_post_compat, sem_post);

typedef struct sem *sem_t;

extern int _libc_sem_init_compat(sem_t *sem, int pshared, unsigned int value);
extern int _libc_sem_destroy_compat(sem_t *sem);
extern int _libc_sem_getvalue_compat(sem_t * __restrict sem, int * __restrict sval);
extern int _libc_sem_trywait_compat(sem_t *sem);
extern int _libc_sem_wait_compat(sem_t *sem);
extern int _libc_sem_timedwait_compat(sem_t * __restrict sem,
    const struct timespec * __restrict abstime);
extern int _libc_sem_post_compat(sem_t *sem);

int _sem_init_compat(sem_t *sem, int pshared, unsigned int value);
int _sem_destroy_compat(sem_t *sem);
int _sem_getvalue_compat(sem_t * __restrict sem, int * __restrict sval);
int _sem_trywait_compat(sem_t *sem);
int _sem_wait_compat(sem_t *sem);
int _sem_timedwait_compat(sem_t * __restrict sem,
    const struct timespec * __restrict abstime);
int _sem_post_compat(sem_t *sem);

int
_sem_init_compat(sem_t *sem, int pshared, unsigned int value)
{
	return _libc_sem_init_compat(sem, pshared, value);
}

int
_sem_destroy_compat(sem_t *sem)
{
	return _libc_sem_destroy_compat(sem);
}

int
_sem_getvalue_compat(sem_t * __restrict sem, int * __restrict sval)
{
	return _libc_sem_getvalue_compat(sem, sval);
}

int
_sem_trywait_compat(sem_t *sem)
{
	return _libc_sem_trywait_compat(sem);
}

int
_sem_wait_compat(sem_t *sem)
{
	return _libc_sem_wait_compat(sem);
}

int
_sem_timedwait_compat(sem_t * __restrict sem,
    const struct timespec * __restrict abstime)
{
	return _libc_sem_timedwait_compat(sem, abstime);
}

int
_sem_post_compat(sem_t *sem)
{
	return _libc_sem_post_compat(sem);
}