
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

#include <assert.h>
#include <err.h>
#include <pthread.h>
#include <stdlib.h>

#include "misc.h"
#include "queue.h"
#include "threads.h"

/*
 * This API is a wrapper around the pthread(3) API, which mainly
 * allows me to wait for multiple threads to exit.  We use a
 * condition variable to signal a thread's death.  All threads
 * created with this API have a common entry/exit point, so we
 * don't need to add any code in the threads themselves.
 */

/* Structure describing a thread. */
struct thread {
	pthread_t thread;
	void *(*start)(void *);
	void *data;
	struct threads *threads;
	LIST_ENTRY(thread) runlist;
	STAILQ_ENTRY(thread) deadlist;
};

/* A set of threads. */
struct threads {
	pthread_mutex_t threads_mtx;
	pthread_cond_t thread_exited;
	LIST_HEAD(, thread) threads_running;
	STAILQ_HEAD(, thread) threads_dead;
};

static void	*thread_start(void *);	/* Common entry point for threads. */

static void	 threads_lock(struct threads *);
static void	 threads_unlock(struct threads *);

static void
threads_lock(struct threads *tds)
{
	int error;

	error = pthread_mutex_lock(&tds->threads_mtx);
	assert(!error);
}

static void
threads_unlock(struct threads *tds)
{
	int error;

	error = pthread_mutex_unlock(&tds->threads_mtx);
	assert(!error);
}

/* Create a new set of threads. */
struct threads *
threads_new(void)
{
	struct threads *tds;

	tds = xmalloc(sizeof(struct threads));
	pthread_mutex_init(&tds->threads_mtx, NULL);
	pthread_cond_init(&tds->thread_exited, NULL);
	LIST_INIT(&tds->threads_running);
	STAILQ_INIT(&tds->threads_dead);
	return (tds);
}

/* Create a new thread in this set. */
void
threads_create(struct threads *tds, void *(*start)(void *), void *data)
{
	pthread_attr_t attr;
	struct thread *td;
	int error;

	td = xmalloc(sizeof(struct thread));
	td->threads = tds;
	td->start = start;
	td->data = data;
	/* We don't use pthread_join() to wait for the threads to finish. */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	threads_lock(tds);
	error = pthread_create(&td->thread, &attr, thread_start, td);
	if (error)
		err(1, "pthread_create");
	LIST_INSERT_HEAD(&tds->threads_running, td, runlist);
	threads_unlock(tds);
}

/* Wait for a thread in the set to exit, and return its data pointer. */
void *
threads_wait(struct threads *tds)
{
	struct thread *td;
	void *data;

	threads_lock(tds);
	while (STAILQ_EMPTY(&tds->threads_dead)) {
		assert(!LIST_EMPTY(&tds->threads_running));
		pthread_cond_wait(&tds->thread_exited, &tds->threads_mtx);
	}
	td = STAILQ_FIRST(&tds->threads_dead);
	STAILQ_REMOVE_HEAD(&tds->threads_dead, deadlist);
	threads_unlock(tds);
	data = td->data;
	free(td);
	return (data);
}

/* Free a threads set. */
void
threads_free(struct threads *tds)
{

	assert(LIST_EMPTY(&tds->threads_running));
	assert(STAILQ_EMPTY(&tds->threads_dead));
	pthread_cond_destroy(&tds->thread_exited);
	pthread_mutex_destroy(&tds->threads_mtx);
	free(tds);
}

/*
 * Common entry point for threads.  This just calls the real start
 * routine, and then signals the thread's death, after having
 * removed the thread from the list.
 */
static void *
thread_start(void *data)
{
	struct threads *tds;
	struct thread *td;

	td = data;
	tds = td->threads;
	td->start(td->data);
	threads_lock(tds);
	LIST_REMOVE(td, runlist);
	STAILQ_INSERT_TAIL(&tds->threads_dead, td, deadlist);
	pthread_cond_signal(&tds->thread_exited);
	threads_unlock(tds);
	return (NULL);
}