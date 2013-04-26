
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

#include <stdlib.h>
#include "thr_private.h"

#define HASHSHIFT	9
#define HASHSIZE	(1 << HASHSHIFT)
#define SC_HASH(wchan) ((unsigned)				\
	((((uintptr_t)(wchan) >> 3)				\
	^ ((uintptr_t)(wchan) >> (HASHSHIFT + 3)))		\
	& (HASHSIZE - 1)))
#define SC_LOOKUP(wc)	&sc_table[SC_HASH(wc)]

struct sleepqueue_chain {
	struct umutex		sc_lock;
	int			sc_enqcnt;
	LIST_HEAD(, sleepqueue) sc_queues;
	int			sc_type;
};

static struct sleepqueue_chain  sc_table[HASHSIZE];

void
_sleepq_init(void)
{
	int	i;

	for (i = 0; i < HASHSIZE; ++i) {
		LIST_INIT(&sc_table[i].sc_queues);
		_thr_umutex_init(&sc_table[i].sc_lock);
	}
}

struct sleepqueue *
_sleepq_alloc(void)
{
	struct sleepqueue *sq;

	sq = calloc(1, sizeof(struct sleepqueue));
	TAILQ_INIT(&sq->sq_blocked);
	SLIST_INIT(&sq->sq_freeq);
	return (sq);
}

void
_sleepq_free(struct sleepqueue *sq)
{
	free(sq);
}

void
_sleepq_lock(void *wchan)
{
	struct pthread *curthread = _get_curthread();
	struct sleepqueue_chain *sc;

	sc = SC_LOOKUP(wchan);
	THR_LOCK_ACQUIRE_SPIN(curthread, &sc->sc_lock);
}

void
_sleepq_unlock(void *wchan)
{
	struct sleepqueue_chain *sc;
	struct pthread *curthread = _get_curthread();
                    
	sc = SC_LOOKUP(wchan);
	THR_LOCK_RELEASE(curthread, &sc->sc_lock);
}

static inline struct sleepqueue *
lookup(struct sleepqueue_chain *sc, void *wchan)
{
	struct sleepqueue *sq;

	LIST_FOREACH(sq, &sc->sc_queues, sq_hash)
		if (sq->sq_wchan == wchan)
			return (sq);
	return (NULL);
}

struct sleepqueue *
_sleepq_lookup(void *wchan)
{
	return (lookup(SC_LOOKUP(wchan), wchan));
}

void
_sleepq_add(void *wchan, struct pthread *td)
{
	struct sleepqueue_chain *sc;
	struct sleepqueue *sq;

	sc = SC_LOOKUP(wchan);
	sq = lookup(sc, wchan);
	if (sq != NULL) {
		SLIST_INSERT_HEAD(&sq->sq_freeq, td->sleepqueue, sq_flink);
	} else {
		sq = td->sleepqueue;
		LIST_INSERT_HEAD(&sc->sc_queues, sq, sq_hash);
		sq->sq_wchan = wchan;
		/* sq->sq_type = type; */
	}
	td->sleepqueue = NULL;
	td->wchan = wchan;
	if (((++sc->sc_enqcnt << _thr_queuefifo) & 0xff) != 0)
		TAILQ_INSERT_HEAD(&sq->sq_blocked, td, wle);
	else
		TAILQ_INSERT_TAIL(&sq->sq_blocked, td, wle);
}

int
_sleepq_remove(struct sleepqueue *sq, struct pthread *td)
{
	int rc;

	TAILQ_REMOVE(&sq->sq_blocked, td, wle);
	if (TAILQ_EMPTY(&sq->sq_blocked)) {
		LIST_REMOVE(sq, sq_hash);
		td->sleepqueue = sq;
		rc = 0;
	} else {
		td->sleepqueue = SLIST_FIRST(&sq->sq_freeq);
		SLIST_REMOVE_HEAD(&sq->sq_freeq, sq_flink);
		rc = 1;
	}
	td->wchan = NULL;
	return (rc);
}

void
_sleepq_drop(struct sleepqueue *sq,
	void (*cb)(struct pthread *, void *arg), void *arg)
{
	struct pthread *td;
	struct sleepqueue *sq2;

	td = TAILQ_FIRST(&sq->sq_blocked);
	if (td == NULL)
		return;
	LIST_REMOVE(sq, sq_hash);
	TAILQ_REMOVE(&sq->sq_blocked, td, wle);
	if (cb != NULL)
		cb(td, arg);
	td->sleepqueue = sq;
	td->wchan = NULL;
	sq2 = SLIST_FIRST(&sq->sq_freeq);
	TAILQ_FOREACH(td, &sq->sq_blocked, wle) {
		if (cb != NULL)
			cb(td, arg);
		td->sleepqueue = sq2;
		td->wchan = NULL;
		sq2 = SLIST_NEXT(sq2, sq_flink);
	}
	TAILQ_INIT(&sq->sq_blocked);
	SLIST_INIT(&sq->sq_freeq);
}