
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
  * A lockless rwlock for rtld.
  */
#include <sys/cdefs.h>
#include <sys/mman.h>
#include <link.h>
#include <stdlib.h>
#include <string.h>

#include "rtld_lock.h"
#include "thr_private.h"

#undef errno
extern int errno;

static int	_thr_rtld_clr_flag(int);
static void	*_thr_rtld_lock_create(void);
static void	_thr_rtld_lock_destroy(void *);
static void	_thr_rtld_lock_release(void *);
static void	_thr_rtld_rlock_acquire(void *);
static int	_thr_rtld_set_flag(int);
static void	_thr_rtld_wlock_acquire(void *);

struct rtld_lock {
	struct	urwlock	lock;
	char		_pad[CACHE_LINE_SIZE - sizeof(struct urwlock)];
};

static struct rtld_lock lock_place[MAX_RTLD_LOCKS] __aligned(CACHE_LINE_SIZE);
static int busy_places;

static void *
_thr_rtld_lock_create(void)
{
	int locki;
	struct rtld_lock *l;
	static const char fail[] = "_thr_rtld_lock_create failed\n";

	for (locki = 0; locki < MAX_RTLD_LOCKS; locki++) {
		if ((busy_places & (1 << locki)) == 0)
			break;
	}
	if (locki == MAX_RTLD_LOCKS) {
		write(2, fail, sizeof(fail) - 1);
		return (NULL);
	}
	busy_places |= (1 << locki);

	l = &lock_place[locki];
	l->lock.rw_flags = URWLOCK_PREFER_READER;
	return (l);
}

static void
_thr_rtld_lock_destroy(void *lock)
{
	int locki;
	size_t i;

	locki = (struct rtld_lock *)lock - &lock_place[0];
	for (i = 0; i < sizeof(struct rtld_lock); ++i)
		((char *)lock)[i] = 0;
	busy_places &= ~(1 << locki);
}

#define SAVE_ERRNO()	{			\
	if (curthread != _thr_initial)		\
		errsave = curthread->error;	\
	else					\
		errsave = errno;		\
}

#define RESTORE_ERRNO()	{ 			\
	if (curthread != _thr_initial)  	\
		curthread->error = errsave;	\
	else					\
		errno = errsave;		\
}

static void
_thr_rtld_rlock_acquire(void *lock)
{
	struct pthread		*curthread;
	struct rtld_lock	*l;
	int			errsave;

	curthread = _get_curthread();
	SAVE_ERRNO();
	l = (struct rtld_lock *)lock;

	THR_CRITICAL_ENTER(curthread);
	while (_thr_rwlock_rdlock(&l->lock, 0, NULL) != 0)
		;
	curthread->rdlock_count++;
	RESTORE_ERRNO();
}

static void
_thr_rtld_wlock_acquire(void *lock)
{
	struct pthread		*curthread;
	struct rtld_lock	*l;
	int			errsave;

	curthread = _get_curthread();
	SAVE_ERRNO();
	l = (struct rtld_lock *)lock;

	THR_CRITICAL_ENTER(curthread);
	while (_thr_rwlock_wrlock(&l->lock, NULL) != 0)
		;
	RESTORE_ERRNO();
}

static void
_thr_rtld_lock_release(void *lock)
{
	struct pthread		*curthread;
	struct rtld_lock	*l;
	int32_t			state;
	int			errsave;

	curthread = _get_curthread();
	SAVE_ERRNO();
	l = (struct rtld_lock *)lock;
	
	state = l->lock.rw_state;
	if (_thr_rwlock_unlock(&l->lock) == 0) {
		if ((state & URWLOCK_WRITE_OWNER) == 0)
			curthread->rdlock_count--;
		THR_CRITICAL_LEAVE(curthread);
	}
	RESTORE_ERRNO();
}

static int
_thr_rtld_set_flag(int mask __unused)
{
	/*
	 * The caller's code in rtld-elf is broken, it is not signal safe,
	 * just return zero to fool it.
	 */
	return (0);
}

static int
_thr_rtld_clr_flag(int mask __unused)
{
	return (0);
}

void
_thr_rtld_init(void)
{
	struct RtldLockInfo	li;
	struct pthread		*curthread;
	long dummy = -1;

	curthread = _get_curthread();

	/* force to resolve _umtx_op PLT */
	_umtx_op_err((struct umtx *)&dummy, UMTX_OP_WAKE, 1, 0, 0);
	
	/* force to resolve errno() PLT */
	__error();

	/* force to resolve memcpy PLT */
	memcpy(&dummy, &dummy, sizeof(dummy));

	mprotect(NULL, 0, 0);
	_rtld_get_stack_prot();

	li.lock_create  = _thr_rtld_lock_create;
	li.lock_destroy = _thr_rtld_lock_destroy;
	li.rlock_acquire = _thr_rtld_rlock_acquire;
	li.wlock_acquire = _thr_rtld_wlock_acquire;
	li.lock_release  = _thr_rtld_lock_release;
	li.thread_set_flag = _thr_rtld_set_flag;
	li.thread_clr_flag = _thr_rtld_clr_flag;
	li.at_fork = NULL;
	
	/* mask signals, also force to resolve __sys_sigprocmask PLT */
	_thr_signal_block(curthread);
	_rtld_thread_init(&li);
	_thr_signal_unblock(curthread);
}