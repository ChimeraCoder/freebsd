
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

__weak_reference(_pthread_barrier_init,		pthread_barrier_init);
__weak_reference(_pthread_barrier_wait,		pthread_barrier_wait);
__weak_reference(_pthread_barrier_destroy,	pthread_barrier_destroy);

int
_pthread_barrier_destroy(pthread_barrier_t *barrier)
{
	pthread_barrier_t	bar;
	int			ret, ret2;

	if (barrier == NULL || *barrier == NULL)
		return (EINVAL);

	bar = *barrier;
	if (bar->b_waiters > 0)
		return (EBUSY);
	*barrier = NULL;
	ret  = _pthread_mutex_destroy(&bar->b_lock);
	ret2 = _pthread_cond_destroy(&bar->b_cond);
	free(bar);
	return (ret ? ret : ret2);
}

int
_pthread_barrier_init(pthread_barrier_t *barrier,
    const pthread_barrierattr_t *attr __unused, unsigned count)
{
	pthread_barrier_t	bar;
	int			ret;

	if (barrier == NULL || count <= 0)
		return (EINVAL);

	bar = malloc(sizeof(struct pthread_barrier));
	if (bar == NULL)
		return (ENOMEM);

	if ((ret = _pthread_mutex_init(&bar->b_lock, NULL)) != 0) {
		free(bar);
		return (ret);
	}

	if ((ret = _pthread_cond_init(&bar->b_cond, NULL)) != 0) {
		_pthread_mutex_destroy(&bar->b_lock);
		free(bar);
		return (ret);
	}

	bar->b_waiters		= 0;
	bar->b_count		= count;
	bar->b_generation	= 0;
	*barrier		= bar;

	return (0);
}

int
_pthread_barrier_wait(pthread_barrier_t *barrier)
{
	int ret, gen;
	pthread_barrier_t bar;

	if (barrier == NULL || *barrier == NULL)
		return (EINVAL);

	bar = *barrier;
	if ((ret = _pthread_mutex_lock(&bar->b_lock)) != 0)
		return (ret);

	if (++bar->b_waiters == bar->b_count) {
		/* Current thread is lastest thread */
		bar->b_generation++;
		bar->b_waiters = 0;
		ret = _pthread_cond_broadcast(&bar->b_cond);
		if (ret == 0)
			ret = PTHREAD_BARRIER_SERIAL_THREAD;
	} else {
		gen = bar->b_generation;
		do {
			ret = _pthread_cond_wait(
				&bar->b_cond, &bar->b_lock);
		/* test generation to avoid bogus wakeup */
		} while (ret == 0 && gen == bar->b_generation);
	}
	_pthread_mutex_unlock(&bar->b_lock);
	return (ret);
}