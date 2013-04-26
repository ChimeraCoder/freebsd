
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <errno.h>
#include <pthread.h>

#include "threads.h"

void
mtx_destroy(mtx_t *mtx)
{

	(void)pthread_mutex_destroy(mtx);
}

int
mtx_init(mtx_t *mtx, int type)
{
	pthread_mutexattr_t attr;
	int mt;

	switch (type) {
	case mtx_plain:
	case mtx_timed:
		mt = PTHREAD_MUTEX_NORMAL;
		break;
	case mtx_plain | mtx_recursive:
	case mtx_timed | mtx_recursive:
		mt = PTHREAD_MUTEX_RECURSIVE;
		break;
	default:
		return (thrd_error);
	}

	if (pthread_mutexattr_init(&attr) != 0)
		return (thrd_error);
	if (pthread_mutexattr_settype(&attr, mt) != 0)
		return (thrd_error);
	if (pthread_mutex_init(mtx, &attr) != 0)
		return (thrd_error);
	return (thrd_success);
}

int
mtx_lock(mtx_t *mtx)
{

	if (pthread_mutex_lock(mtx) != 0)
		return (thrd_error);
	return (thrd_success);
}

int
mtx_timedlock(mtx_t *restrict mtx, const struct timespec *restrict ts)
{

	switch (pthread_mutex_timedlock(mtx, ts)) {
	case 0:
		return (thrd_success);
	case ETIMEDOUT:
		return (thrd_timedout);
	default:
		return (thrd_error);
	}
}

int
mtx_trylock(mtx_t *mtx)
{

	switch (pthread_mutex_lock(mtx)) {
	case 0:
		return (thrd_success);
	case EBUSY:
		return (thrd_busy);
	default:
		return (thrd_error);
	}
}

int
mtx_unlock(mtx_t *mtx)
{

	if (pthread_mutex_unlock(mtx) != 0)
		return (thrd_error);
	return (thrd_success);
}