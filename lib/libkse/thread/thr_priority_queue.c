
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
#include <stdlib.h>
#include <sys/queue.h>
#include <string.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

/* Prototypes: */
static void pq_insert_prio_list(pq_queue_t *pq, int prio);

#if defined(_PTHREADS_INVARIANTS)

#define PQ_IN_SCHEDQ	(THR_FLAGS_IN_RUNQ | THR_FLAGS_IN_WAITQ)

#define PQ_SET_ACTIVE(pq)		(pq)->pq_flags |= PQF_ACTIVE
#define PQ_CLEAR_ACTIVE(pq)		(pq)->pq_flags &= ~PQF_ACTIVE
#define PQ_ASSERT_ACTIVE(pq, msg)	do {		\
	if (((pq)->pq_flags & PQF_ACTIVE) == 0)		\
		PANIC(msg);				\
} while (0)
#define PQ_ASSERT_INACTIVE(pq, msg)	do {		\
	if (((pq)->pq_flags & PQF_ACTIVE) != 0)		\
		PANIC(msg);				\
} while (0)
#define PQ_ASSERT_IN_WAITQ(thrd, msg)	do {		\
	if (((thrd)->flags & THR_FLAGS_IN_WAITQ) == 0) \
		PANIC(msg);				\
} while (0)
#define PQ_ASSERT_IN_RUNQ(thrd, msg)	do {		\
	if (((thrd)->flags & THR_FLAGS_IN_RUNQ) == 0) \
		PANIC(msg);				\
} while (0)
#define PQ_ASSERT_NOT_QUEUED(thrd, msg) do {		\
	if (((thrd)->flags & PQ_IN_SCHEDQ) != 0)	\
		PANIC(msg);				\
} while (0)

#else

#define PQ_SET_ACTIVE(pq)
#define PQ_CLEAR_ACTIVE(pq)
#define PQ_ASSERT_ACTIVE(pq, msg)
#define PQ_ASSERT_INACTIVE(pq, msg)
#define PQ_ASSERT_IN_WAITQ(thrd, msg)
#define PQ_ASSERT_IN_RUNQ(thrd, msg)
#define PQ_ASSERT_NOT_QUEUED(thrd, msg)

#endif

int
_pq_alloc(pq_queue_t *pq, int minprio, int maxprio)
{
	int ret = 0;
	int prioslots = maxprio - minprio + 1;

	if (pq == NULL)
		ret = -1;

	/* Create the priority queue with (maxprio - minprio + 1) slots: */
	else if	((pq->pq_lists =
	    (pq_list_t *) malloc(sizeof(pq_list_t) * prioslots)) == NULL)
		ret = -1;

	else {
		/* Remember the queue size: */
		pq->pq_size = prioslots;
		ret = _pq_init(pq);
	}
	return (ret);
}

void
_pq_free(pq_queue_t *pq)
{
	if ((pq != NULL) && (pq->pq_lists != NULL))
		free(pq->pq_lists);
}

int
_pq_init(pq_queue_t *pq)
{
	int i, ret = 0;

	if ((pq == NULL) || (pq->pq_lists == NULL))
		ret = -1;

	else {
		/* Initialize the queue for each priority slot: */
		for (i = 0; i < pq->pq_size; i++) {
			TAILQ_INIT(&pq->pq_lists[i].pl_head);
			pq->pq_lists[i].pl_prio = i;
			pq->pq_lists[i].pl_queued = 0;
		}
		/* Initialize the priority queue: */
		TAILQ_INIT(&pq->pq_queue);
		pq->pq_flags = 0;
		pq->pq_threads = 0;
	}
	return (ret);
}

void
_pq_remove(pq_queue_t *pq, pthread_t pthread)
{
	int prio = pthread->active_priority;

	/*
	 * Make some assertions when debugging is enabled:
	 */
	PQ_ASSERT_INACTIVE(pq, "_pq_remove: pq_active");
	PQ_SET_ACTIVE(pq);
	PQ_ASSERT_IN_RUNQ(pthread, "_pq_remove: Not in priority queue");

	/*
	 * Remove this thread from priority list.  Note that if
	 * the priority list becomes empty, it is not removed
	 * from the priority queue because another thread may be
	 * added to the priority list (resulting in a needless
	 * removal/insertion).  Priority lists are only removed
	 * from the priority queue when _pq_first is called.
	 */
	TAILQ_REMOVE(&pq->pq_lists[prio].pl_head, pthread, pqe);
	pq->pq_threads--;
	/* This thread is now longer in the priority queue. */
	pthread->flags &= ~THR_FLAGS_IN_RUNQ;
	
	PQ_CLEAR_ACTIVE(pq);
}


void
_pq_insert_head(pq_queue_t *pq, pthread_t pthread)
{
	int prio;

	/*
	 * Make some assertions when debugging is enabled:
	 */
	PQ_ASSERT_INACTIVE(pq, "_pq_insert_head: pq_active");
	PQ_SET_ACTIVE(pq);
	PQ_ASSERT_NOT_QUEUED(pthread,
	    "_pq_insert_head: Already in priority queue");

	prio = pthread->active_priority;
	TAILQ_INSERT_HEAD(&pq->pq_lists[prio].pl_head, pthread, pqe);
	if (pq->pq_lists[prio].pl_queued == 0)
		/* Insert the list into the priority queue: */
		pq_insert_prio_list(pq, prio);
	pq->pq_threads++;
	/* Mark this thread as being in the priority queue. */
	pthread->flags |= THR_FLAGS_IN_RUNQ;

	PQ_CLEAR_ACTIVE(pq);
}


void
_pq_insert_tail(pq_queue_t *pq, pthread_t pthread)
{
	int prio;

	/*
	 * Make some assertions when debugging is enabled:
	 */
	PQ_ASSERT_INACTIVE(pq, "_pq_insert_tail: pq_active");
	PQ_SET_ACTIVE(pq);
	PQ_ASSERT_NOT_QUEUED(pthread,
	    "_pq_insert_tail: Already in priority queue");

	prio = pthread->active_priority;
	TAILQ_INSERT_TAIL(&pq->pq_lists[prio].pl_head, pthread, pqe);
	if (pq->pq_lists[prio].pl_queued == 0)
		/* Insert the list into the priority queue: */
		pq_insert_prio_list(pq, prio);
	pq->pq_threads++;
	/* Mark this thread as being in the priority queue. */
	pthread->flags |= THR_FLAGS_IN_RUNQ;

	PQ_CLEAR_ACTIVE(pq);
}


pthread_t
_pq_first(pq_queue_t *pq)
{
	pq_list_t *pql;
	pthread_t pthread = NULL;

	/*
	 * Make some assertions when debugging is enabled:
	 */
	PQ_ASSERT_INACTIVE(pq, "_pq_first: pq_active");
	PQ_SET_ACTIVE(pq);

	while (((pql = TAILQ_FIRST(&pq->pq_queue)) != NULL) &&
	    (pthread == NULL)) {
		if ((pthread = TAILQ_FIRST(&pql->pl_head)) == NULL) {
			/*
			 * The priority list is empty; remove the list
			 * from the queue.
			 */
			TAILQ_REMOVE(&pq->pq_queue, pql, pl_link);

			/* Mark the list as not being in the queue: */
			pql->pl_queued = 0;
		}
	}

	PQ_CLEAR_ACTIVE(pq);
	return (pthread);
}

/*
 * Select a thread which is allowed to run by debugger, we probably
 * should merge the function into _pq_first if that function is only
 * used by scheduler to select a thread.
 */
pthread_t
_pq_first_debug(pq_queue_t *pq)
{
	pq_list_t *pql, *pqlnext = NULL;
	pthread_t pthread = NULL;

	/*
	 * Make some assertions when debugging is enabled:
	 */
	PQ_ASSERT_INACTIVE(pq, "_pq_first: pq_active");
	PQ_SET_ACTIVE(pq);

	for (pql = TAILQ_FIRST(&pq->pq_queue);
	     pql != NULL && pthread == NULL; pql = pqlnext) {
		if ((pthread = TAILQ_FIRST(&pql->pl_head)) == NULL) {
			/*
			 * The priority list is empty; remove the list
			 * from the queue.
			 */
			pqlnext = TAILQ_NEXT(pql, pl_link);
			TAILQ_REMOVE(&pq->pq_queue, pql, pl_link);

			/* Mark the list as not being in the queue: */
			pql->pl_queued = 0;
		} else {
			/*
			 * note there may be a suspension event during this
			 * test, If TMDF_SUSPEND is set after we tested it,
			 * we will run the thread, this seems be a problem,
			 * fortunatly, when we are being debugged, all context
			 * switch will be done by kse_switchin, that is a
			 * syscall, kse_switchin will check the flag again,
			 * the thread will be returned via upcall, so next
			 * time, UTS won't run the thread.
			 */ 
			while (pthread != NULL && !DBG_CAN_RUN(pthread)) {
				pthread = TAILQ_NEXT(pthread, pqe);
			}
			if (pthread == NULL)
				pqlnext = TAILQ_NEXT(pql, pl_link);
		}
	}

	PQ_CLEAR_ACTIVE(pq);
	return (pthread);
}

static void
pq_insert_prio_list(pq_queue_t *pq, int prio)
{
	pq_list_t *pql;

	/*
	 * Make some assertions when debugging is enabled:
	 */
	PQ_ASSERT_ACTIVE(pq, "pq_insert_prio_list: pq_active");

	/*
	 * The priority queue is in descending priority order.  Start at
	 * the beginning of the queue and find the list before which the
	 * new list should be inserted.
	 */
	pql = TAILQ_FIRST(&pq->pq_queue);
	while ((pql != NULL) && (pql->pl_prio > prio))
		pql = TAILQ_NEXT(pql, pl_link);

	/* Insert the list: */
	if (pql == NULL)
		TAILQ_INSERT_TAIL(&pq->pq_queue, &pq->pq_lists[prio], pl_link);
	else
		TAILQ_INSERT_BEFORE(pql, &pq->pq_lists[prio], pl_link);

	/* Mark this list as being in the queue: */
	pq->pq_lists[prio].pl_queued = 1;
}