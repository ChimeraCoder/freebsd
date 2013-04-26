
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/rangelock.h>
#include <sys/systm.h>

#include <vm/uma.h>

struct rl_q_entry {
	TAILQ_ENTRY(rl_q_entry) rl_q_link;
	off_t		rl_q_start, rl_q_end;
	int		rl_q_flags;
};

static uma_zone_t rl_entry_zone;

static void
rangelock_sys_init(void)
{

	rl_entry_zone = uma_zcreate("rl_entry", sizeof(struct rl_q_entry),
	    NULL, NULL, NULL, NULL, UMA_ALIGN_PTR, 0);
}
SYSINIT(vfs, SI_SUB_LOCK, SI_ORDER_ANY, rangelock_sys_init, NULL);

static struct rl_q_entry *
rlqentry_alloc(void)
{

	return (uma_zalloc(rl_entry_zone, M_WAITOK));
}

void
rlqentry_free(struct rl_q_entry *rleq)
{

	uma_zfree(rl_entry_zone, rleq);
}

void
rangelock_init(struct rangelock *lock)
{

	TAILQ_INIT(&lock->rl_waiters);
	lock->rl_currdep = NULL;
}

void
rangelock_destroy(struct rangelock *lock)
{

	KASSERT(TAILQ_EMPTY(&lock->rl_waiters), ("Dangling waiters"));
}

/*
 * Verifies the supplied rl_q_entries for compatibility.  Returns true
 * if the rangelock queue entries are not compatible, false if they are.
 *
 * Two entries are compatible if their ranges do not overlap, or both
 * entries are for read.
 */
static int
rangelock_incompatible(const struct rl_q_entry *e1,
    const struct rl_q_entry *e2)
{

	if ((e1->rl_q_flags & RL_LOCK_TYPE_MASK) == RL_LOCK_READ &&
	    (e2->rl_q_flags & RL_LOCK_TYPE_MASK) == RL_LOCK_READ)
		return (0);
	if (e1->rl_q_start < e2->rl_q_end && e1->rl_q_end > e2->rl_q_start)
		return (1);
	return (0);
}

/*
 * Recalculate the lock->rl_currdep after an unlock.
 */
static void
rangelock_calc_block(struct rangelock *lock)
{
	struct rl_q_entry *entry, *entry1, *whead;

	if (lock->rl_currdep == TAILQ_FIRST(&lock->rl_waiters) &&
	    lock->rl_currdep != NULL)
		lock->rl_currdep = TAILQ_NEXT(lock->rl_currdep, rl_q_link);
	for (entry = lock->rl_currdep; entry != NULL;
	     entry = TAILQ_NEXT(entry, rl_q_link)) {
		TAILQ_FOREACH(entry1, &lock->rl_waiters, rl_q_link) {
			if (rangelock_incompatible(entry, entry1))
				goto out;
			if (entry1 == entry)
				break;
		}
	}
out:
	lock->rl_currdep = entry;
	TAILQ_FOREACH(whead, &lock->rl_waiters, rl_q_link) {
		if (whead == lock->rl_currdep)
			break;
		if (!(whead->rl_q_flags & RL_LOCK_GRANTED)) {
			whead->rl_q_flags |= RL_LOCK_GRANTED;
			wakeup(whead);
		}
	}
}

static void
rangelock_unlock_locked(struct rangelock *lock, struct rl_q_entry *entry,
    struct mtx *ilk)
{

	MPASS(lock != NULL && entry != NULL && ilk != NULL);
	mtx_assert(ilk, MA_OWNED);
	KASSERT(entry != lock->rl_currdep, ("stuck currdep"));

	TAILQ_REMOVE(&lock->rl_waiters, entry, rl_q_link);
	rangelock_calc_block(lock);
	mtx_unlock(ilk);
	if (curthread->td_rlqe == NULL)
		curthread->td_rlqe = entry;
	else
		rlqentry_free(entry);
}

void
rangelock_unlock(struct rangelock *lock, void *cookie, struct mtx *ilk)
{

	MPASS(lock != NULL && cookie != NULL && ilk != NULL);

	mtx_lock(ilk);
	rangelock_unlock_locked(lock, cookie, ilk);
}

/*
 * Unlock the sub-range of granted lock.
 */
void *
rangelock_unlock_range(struct rangelock *lock, void *cookie, off_t start,
    off_t end, struct mtx *ilk)
{
	struct rl_q_entry *entry;

	MPASS(lock != NULL && cookie != NULL && ilk != NULL);
	entry = cookie;
	KASSERT(entry->rl_q_flags & RL_LOCK_GRANTED,
	    ("Unlocking non-granted lock"));
	KASSERT(entry->rl_q_start == start, ("wrong start"));
	KASSERT(entry->rl_q_end >= end, ("wrong end"));

	mtx_lock(ilk);
	if (entry->rl_q_end == end) {
		rangelock_unlock_locked(lock, cookie, ilk);
		return (NULL);
	}
	entry->rl_q_end = end;
	rangelock_calc_block(lock);
	mtx_unlock(ilk);
	return (cookie);
}

/*
 * Add the lock request to the queue of the pending requests for
 * rangelock.  Sleep until the request can be granted.
 */
static void *
rangelock_enqueue(struct rangelock *lock, off_t start, off_t end, int mode,
    struct mtx *ilk)
{
	struct rl_q_entry *entry;
	struct thread *td;

	MPASS(lock != NULL && ilk != NULL);

	td = curthread;
	if (td->td_rlqe != NULL) {
		entry = td->td_rlqe;
		td->td_rlqe = NULL;
	} else
		entry = rlqentry_alloc();
	MPASS(entry != NULL);
	entry->rl_q_flags = mode;
	entry->rl_q_start = start;
	entry->rl_q_end = end;

	mtx_lock(ilk);
	/*
	 * XXXKIB TODO. Check that a thread does not try to enqueue a
	 * lock that is incompatible with another request from the same
	 * thread.
	 */

	TAILQ_INSERT_TAIL(&lock->rl_waiters, entry, rl_q_link);
	if (lock->rl_currdep == NULL)
		lock->rl_currdep = entry;
	rangelock_calc_block(lock);
	while (!(entry->rl_q_flags & RL_LOCK_GRANTED))
		msleep(entry, ilk, 0, "range", 0);
	mtx_unlock(ilk);
	return (entry);
}

void *
rangelock_rlock(struct rangelock *lock, off_t start, off_t end, struct mtx *ilk)
{

	return (rangelock_enqueue(lock, start, end, RL_LOCK_READ, ilk));
}

void *
rangelock_wlock(struct rangelock *lock, off_t start, off_t end, struct mtx *ilk)
{

	return (rangelock_enqueue(lock, start, end, RL_LOCK_WRITE, ilk));
}