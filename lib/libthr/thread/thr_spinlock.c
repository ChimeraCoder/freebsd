
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

#include <sys/types.h>
#include <pthread.h>
#include <libc_private.h>
#include <spinlock.h>

#include "thr_private.h"

#define	MAX_SPINLOCKS	72

/*
 * These data structures are used to trace all spinlocks
 * in libc.
 */
struct spinlock_extra {
	spinlock_t	*owner;
	struct umutex	lock;
};

static struct umutex		spinlock_static_lock = DEFAULT_UMUTEX;
static struct spinlock_extra	extra[MAX_SPINLOCKS];
static int			spinlock_count;
static int			initialized;

static void	init_spinlock(spinlock_t *lck);

/*
 * These are for compatability only.  Spinlocks of this type
 * are deprecated.
 */

void
_spinunlock(spinlock_t *lck)
{
	struct spinlock_extra	*_extra;

	_extra = (struct spinlock_extra *)lck->fname;
	THR_UMUTEX_UNLOCK(_get_curthread(), &_extra->lock);
}

void
_spinlock(spinlock_t *lck)
{
	struct spinlock_extra *_extra;

	if (!__isthreaded)
		PANIC("Spinlock called when not threaded.");
	if (!initialized)
		PANIC("Spinlocks not initialized.");
	if (lck->fname == NULL)
		init_spinlock(lck);
	_extra = (struct spinlock_extra *)lck->fname;
	THR_UMUTEX_LOCK(_get_curthread(), &_extra->lock);
}

void
_spinlock_debug(spinlock_t *lck, char *fname __unused, int lineno __unused)
{
	_spinlock(lck);
}

static void
init_spinlock(spinlock_t *lck)
{
	struct pthread *curthread = _get_curthread();

	THR_UMUTEX_LOCK(curthread, &spinlock_static_lock);
	if ((lck->fname == NULL) && (spinlock_count < MAX_SPINLOCKS)) {
		lck->fname = (char *)&extra[spinlock_count];
		_thr_umutex_init(&extra[spinlock_count].lock);
		extra[spinlock_count].owner = lck;
		spinlock_count++;
	}
	THR_UMUTEX_UNLOCK(curthread, &spinlock_static_lock);
	if (lck->fname == NULL)
		PANIC("Warning: exceeded max spinlocks");
}

void
_thr_spinlock_init(void)
{
	int i;

	_thr_umutex_init(&spinlock_static_lock);
	if (initialized != 0) {
		/*
		 * called after fork() to reset state of libc spin locks,
		 * it is not quite right since libc may be in inconsistent
		 * state, resetting the locks to allow current thread to be
		 * able to hold them may not help things too much, but
		 * anyway, we do our best.
		 * it is better to do pthread_atfork in libc.
		 */
		for (i = 0; i < spinlock_count; i++)
			_thr_umutex_init(&extra[i].lock);
	} else {
		initialized = 1;
	}
}