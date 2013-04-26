
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * This file implements a barrier, a synchronization primitive designed to allow
 * threads to wait for each other at given points.  Barriers are initialized
 * with a given number of threads, n, using barrier_init().  When a thread calls
 * barrier_wait(), that thread blocks until n - 1 other threads reach the
 * barrier_wait() call using the same barrier_t.  When n threads have reached
 * the barrier, they are all awakened and sent on their way.  One of the threads
 * returns from barrier_wait() with a return code of 1; the remaining threads
 * get a return code of 0.
 */

#include <pthread.h>
#if defined(sun)
#include <synch.h>
#endif
#include <stdio.h>

#include "barrier.h"

void
barrier_init(barrier_t *bar, int nthreads)
{
	pthread_mutex_init(&bar->bar_lock, NULL);
#if defined(sun)
	sema_init(&bar->bar_sem, 0, USYNC_THREAD, NULL);
#else
	sem_init(&bar->bar_sem, 0, 0);
#endif

	bar->bar_numin = 0;
	bar->bar_nthr = nthreads;
}

int
barrier_wait(barrier_t *bar)
{
	pthread_mutex_lock(&bar->bar_lock);

	if (++bar->bar_numin < bar->bar_nthr) {
		pthread_mutex_unlock(&bar->bar_lock);
#if defined(sun)
		sema_wait(&bar->bar_sem);
#else
		sem_wait(&bar->bar_sem);
#endif

		return (0);

	} else {
		int i;

		/* reset for next use */
		bar->bar_numin = 0;
		for (i = 1; i < bar->bar_nthr; i++)
#if defined(sun)
			sema_post(&bar->bar_sem);
#else
			sem_post(&bar->bar_sem);
#endif
		pthread_mutex_unlock(&bar->bar_lock);

		return (1);
	}
}