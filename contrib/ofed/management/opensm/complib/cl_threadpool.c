
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

/*
 * Abstract:
 *	Implementation of thread pool.
 *
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <complib/cl_threadpool.h>

static void cleanup_mutex(void *arg)
{
	pthread_mutex_unlock(&((cl_thread_pool_t *) arg)->mutex);
}

static void *thread_pool_routine(void *context)
{
	cl_thread_pool_t *p_thread_pool = (cl_thread_pool_t *) context;

	do {
		pthread_mutex_lock(&p_thread_pool->mutex);
		pthread_cleanup_push(cleanup_mutex, p_thread_pool);
		while (!p_thread_pool->events)
			pthread_cond_wait(&p_thread_pool->cond,
					  &p_thread_pool->mutex);
		p_thread_pool->events--;
		pthread_cleanup_pop(1);
		/* The event has been signalled.  Invoke the callback. */
		(*p_thread_pool->pfn_callback) (p_thread_pool->context);
	} while (1);

	return NULL;
}

cl_status_t
cl_thread_pool_init(IN cl_thread_pool_t * const p_thread_pool,
		    IN unsigned count,
		    IN void (*pfn_callback) (void *),
		    IN void *context, IN const char *const name)
{
	int i;

	CL_ASSERT(p_thread_pool);
	CL_ASSERT(pfn_callback);

	memset(p_thread_pool, 0, sizeof(*p_thread_pool));

	if (!count)
		count = cl_proc_count();

	pthread_mutex_init(&p_thread_pool->mutex, NULL);
	pthread_cond_init(&p_thread_pool->cond, NULL);

	p_thread_pool->events = 0;

	p_thread_pool->pfn_callback = pfn_callback;
	p_thread_pool->context = context;

	p_thread_pool->tid = calloc(count, sizeof(*p_thread_pool->tid));
	if (!p_thread_pool->tid) {
		cl_thread_pool_destroy(p_thread_pool);
		return CL_INSUFFICIENT_MEMORY;
	}

	p_thread_pool->running_count = count;

	for (i = 0; i < count; i++) {
		if (pthread_create(&p_thread_pool->tid[i], NULL,
				   thread_pool_routine, p_thread_pool) < 0) {
			cl_thread_pool_destroy(p_thread_pool);
			return CL_INSUFFICIENT_RESOURCES;
		}
	}

	return (CL_SUCCESS);
}

void cl_thread_pool_destroy(IN cl_thread_pool_t * const p_thread_pool)
{
	int i;

	CL_ASSERT(p_thread_pool);

	for (i = 0; i < p_thread_pool->running_count; i++)
		if (p_thread_pool->tid[i])
			pthread_cancel(p_thread_pool->tid[i]);

	for (i = 0; i < p_thread_pool->running_count; i++)
		if (p_thread_pool->tid[i])
			pthread_join(p_thread_pool->tid[i], NULL);

	p_thread_pool->running_count = 0;
	pthread_cond_destroy(&p_thread_pool->cond);
	pthread_mutex_destroy(&p_thread_pool->mutex);

	p_thread_pool->events = 0;
}

cl_status_t cl_thread_pool_signal(IN cl_thread_pool_t * const p_thread_pool)
{
	int ret;
	CL_ASSERT(p_thread_pool);
	pthread_mutex_lock(&p_thread_pool->mutex);
	p_thread_pool->events++;
	ret = pthread_cond_signal(&p_thread_pool->cond);
	pthread_mutex_unlock(&p_thread_pool->mutex);
	return ret;
}