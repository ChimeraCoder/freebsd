
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
#include <machine/atomic.h>
#include <pthread.h>
#include <libc_private.h>
#include "un-namespace.h"
#include "spinlock.h"
#include "thr_private.h"

#define	MAX_SPINLOCKS	72

struct spinlock_extra {
	spinlock_t	*owner;
	pthread_mutex_t	lock;
};

struct nv_spinlock {
	long   access_lock;
	long   lock_owner;
	struct spinlock_extra *extra;	/* overlays fname in spinlock_t */
	int    lineno;
};
typedef struct nv_spinlock nv_spinlock_t;

static void	init_spinlock(spinlock_t *lck);

static struct pthread_mutex_attr static_mutex_attr =
    PTHREAD_MUTEXATTR_STATIC_INITIALIZER;
static pthread_mutexattr_t	static_mattr = &static_mutex_attr;

static pthread_mutex_t		spinlock_static_lock;
static struct spinlock_extra	extra[MAX_SPINLOCKS];
static int			spinlock_count = 0;
static int			initialized = 0;

/*
 * These are for compatability only.  Spinlocks of this type
 * are deprecated.
 */

void
_spinunlock(spinlock_t *lck)
{
	struct spinlock_extra *sl_extra;

	sl_extra = ((nv_spinlock_t *)lck)->extra;
	_pthread_mutex_unlock(&sl_extra->lock);
}

/*
 * Lock a location for the running thread. Yield to allow other
 * threads to run if this thread is blocked because the lock is
 * not available. Note that this function does not sleep. It
 * assumes that the lock will be available very soon.
 */
void
_spinlock(spinlock_t *lck)
{
	struct spinlock_extra *sl_extra;

	if (!__isthreaded)
		PANIC("Spinlock called when not threaded.");
	if (!initialized)
		PANIC("Spinlocks not initialized.");
	/*
	 * Try to grab the lock and loop if another thread grabs
	 * it before we do.
	 */
	if (lck->fname == NULL)
		init_spinlock(lck);
	sl_extra = ((nv_spinlock_t *)lck)->extra;
	_pthread_mutex_lock(&sl_extra->lock);
}

/*
 * Lock a location for the running thread. Yield to allow other
 * threads to run if this thread is blocked because the lock is
 * not available. Note that this function does not sleep. It
 * assumes that the lock will be available very soon.
 *
 * This function checks if the running thread has already locked the
 * location, warns if this occurs and creates a thread dump before
 * returning.
 */
void
_spinlock_debug(spinlock_t *lck, char *fname __unused, int lineno __unused)
{
	_spinlock(lck);
}

static void
init_spinlock(spinlock_t *lck)
{
	_pthread_mutex_lock(&spinlock_static_lock);
	if ((lck->fname == NULL) && (spinlock_count < MAX_SPINLOCKS)) {
		lck->fname = (char *)&extra[spinlock_count];
		extra[spinlock_count].owner = lck;
		spinlock_count++;
	}
	_pthread_mutex_unlock(&spinlock_static_lock);
	if (lck->fname == NULL)
		PANIC("Exceeded max spinlocks");
}

void
_thr_spinlock_init(void)
{
	int i;

	if (initialized != 0) {
		_thr_mutex_reinit(&spinlock_static_lock);
		for (i = 0; i < spinlock_count; i++)
			_thr_mutex_reinit(&extra[i].lock);
	} else {
		if (_pthread_mutex_init(&spinlock_static_lock, &static_mattr))
			PANIC("Cannot initialize spinlock_static_lock");
		for (i = 0; i < MAX_SPINLOCKS; i++) {
			if (_pthread_mutex_init(&extra[i].lock, &static_mattr))
				PANIC("Cannot initialize spinlock extra");
		}
		initialized = 1;
	}
}