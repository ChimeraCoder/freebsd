
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

#include <dev/drm2/ttm/ttm_lock.h>
#include <dev/drm2/ttm/ttm_module.h>

#define TTM_WRITE_LOCK_PENDING    (1 << 0)
#define TTM_VT_LOCK_PENDING       (1 << 1)
#define TTM_SUSPEND_LOCK_PENDING  (1 << 2)
#define TTM_VT_LOCK               (1 << 3)
#define TTM_SUSPEND_LOCK          (1 << 4)

void ttm_lock_init(struct ttm_lock *lock)
{
	mtx_init(&lock->lock, "ttmlk", NULL, MTX_DEF);
	lock->rw = 0;
	lock->flags = 0;
	lock->kill_takers = false;
	lock->signal = SIGKILL;
}

static void
ttm_lock_send_sig(int signo)
{
	struct proc *p;

	p = curproc;	/* XXXKIB curthread ? */
	PROC_LOCK(p);
	kern_psignal(p, signo);
	PROC_UNLOCK(p);
}

void ttm_read_unlock(struct ttm_lock *lock)
{
	mtx_lock(&lock->lock);
	if (--lock->rw == 0)
		wakeup(lock);
	mtx_unlock(&lock->lock);
}

static bool __ttm_read_lock(struct ttm_lock *lock)
{
	bool locked = false;

	if (unlikely(lock->kill_takers)) {
		ttm_lock_send_sig(lock->signal);
		return false;
	}
	if (lock->rw >= 0 && lock->flags == 0) {
		++lock->rw;
		locked = true;
	}
	return locked;
}

int
ttm_read_lock(struct ttm_lock *lock, bool interruptible)
{
	const char *wmsg;
	int flags, ret;

	ret = 0;
	if (interruptible) {
		flags = PCATCH;
		wmsg = "ttmri";
	} else {
		flags = 0;
		wmsg = "ttmr";
	}
	mtx_lock(&lock->lock);
	while (!__ttm_read_lock(lock)) {
		ret = msleep(lock, &lock->lock, flags, wmsg, 0);
		if (ret != 0)
			break;
	}
	return (-ret);
}

static bool __ttm_read_trylock(struct ttm_lock *lock, bool *locked)
{
	bool block = true;

	*locked = false;

	if (unlikely(lock->kill_takers)) {
		ttm_lock_send_sig(lock->signal);
		return false;
	}
	if (lock->rw >= 0 && lock->flags == 0) {
		++lock->rw;
		block = false;
		*locked = true;
	} else if (lock->flags == 0) {
		block = false;
	}

	return !block;
}

int ttm_read_trylock(struct ttm_lock *lock, bool interruptible)
{
	const char *wmsg;
	int flags, ret;
	bool locked;

	ret = 0;
	if (interruptible) {
		flags = PCATCH;
		wmsg = "ttmrti";
	} else {
		flags = 0;
		wmsg = "ttmrt";
	}
	mtx_lock(&lock->lock);
	while (!__ttm_read_trylock(lock, &locked)) {
		ret = msleep(lock, &lock->lock, flags, wmsg, 0);
		if (ret != 0)
			break;
	}
	MPASS(!locked || ret == 0);
	mtx_unlock(&lock->lock);

	return (locked) ? 0 : -EBUSY;
}

void ttm_write_unlock(struct ttm_lock *lock)
{
	mtx_lock(&lock->lock);
	lock->rw = 0;
	wakeup(lock);
	mtx_unlock(&lock->lock);
}

static bool __ttm_write_lock(struct ttm_lock *lock)
{
	bool locked = false;

	if (unlikely(lock->kill_takers)) {
		ttm_lock_send_sig(lock->signal);
		return false;
	}
	if (lock->rw == 0 && ((lock->flags & ~TTM_WRITE_LOCK_PENDING) == 0)) {
		lock->rw = -1;
		lock->flags &= ~TTM_WRITE_LOCK_PENDING;
		locked = true;
	} else {
		lock->flags |= TTM_WRITE_LOCK_PENDING;
	}
	return locked;
}

int
ttm_write_lock(struct ttm_lock *lock, bool interruptible)
{
	const char *wmsg;
	int flags, ret;

	ret = 0;
	if (interruptible) {
		flags = PCATCH;
		wmsg = "ttmwi";
	} else {
		flags = 0;
		wmsg = "ttmw";
	}
	mtx_lock(&lock->lock);
	/* XXXKIB: linux uses __ttm_read_lock for uninterruptible sleeps */
	while (!__ttm_write_lock(lock)) {
		ret = msleep(lock, &lock->lock, flags, wmsg, 0);
		if (interruptible && ret != 0) {
			lock->flags &= ~TTM_WRITE_LOCK_PENDING;
			wakeup(lock);
			break;
		}
	}
	mtx_unlock(&lock->lock);

	return (-ret);
}

void ttm_write_lock_downgrade(struct ttm_lock *lock)
{
	mtx_lock(&lock->lock);
	lock->rw = 1;
	wakeup(lock);
	mtx_unlock(&lock->lock);
}

static int __ttm_vt_unlock(struct ttm_lock *lock)
{
	int ret = 0;

	mtx_lock(&lock->lock);
	if (unlikely(!(lock->flags & TTM_VT_LOCK)))
		ret = -EINVAL;
	lock->flags &= ~TTM_VT_LOCK;
	wakeup(lock);
	mtx_unlock(&lock->lock);

	return ret;
}

static void ttm_vt_lock_remove(struct ttm_base_object **p_base)
{
	struct ttm_base_object *base = *p_base;
	struct ttm_lock *lock = container_of(base, struct ttm_lock, base);
	int ret;

	*p_base = NULL;
	ret = __ttm_vt_unlock(lock);
	MPASS(ret == 0);
}

static bool __ttm_vt_lock(struct ttm_lock *lock)
{
	bool locked = false;

	if (lock->rw == 0) {
		lock->flags &= ~TTM_VT_LOCK_PENDING;
		lock->flags |= TTM_VT_LOCK;
		locked = true;
	} else {
		lock->flags |= TTM_VT_LOCK_PENDING;
	}
	return locked;
}

int ttm_vt_lock(struct ttm_lock *lock,
		bool interruptible,
		struct ttm_object_file *tfile)
{
	const char *wmsg;
	int flags, ret;

	ret = 0;
	if (interruptible) {
		flags = PCATCH;
		wmsg = "ttmwi";
	} else {
		flags = 0;
		wmsg = "ttmw";
	}
	mtx_lock(&lock->lock);
	while (!__ttm_vt_lock(lock)) {
		ret = msleep(lock, &lock->lock, flags, wmsg, 0);
		if (interruptible && ret != 0) {
			lock->flags &= ~TTM_VT_LOCK_PENDING;
			wakeup(lock);
			break;
		}
	}

	/*
	 * Add a base-object, the destructor of which will
	 * make sure the lock is released if the client dies
	 * while holding it.
	 */

	ret = ttm_base_object_init(tfile, &lock->base, false,
				   ttm_lock_type, &ttm_vt_lock_remove, NULL);
	if (ret)
		(void)__ttm_vt_unlock(lock);
	else
		lock->vt_holder = tfile;

	return (-ret);
}

int ttm_vt_unlock(struct ttm_lock *lock)
{
	return ttm_ref_object_base_unref(lock->vt_holder,
					 lock->base.hash.key, TTM_REF_USAGE);
}

void ttm_suspend_unlock(struct ttm_lock *lock)
{
	mtx_lock(&lock->lock);
	lock->flags &= ~TTM_SUSPEND_LOCK;
	wakeup(lock);
	mtx_unlock(&lock->lock);
}

static bool __ttm_suspend_lock(struct ttm_lock *lock)
{
	bool locked = false;

	if (lock->rw == 0) {
		lock->flags &= ~TTM_SUSPEND_LOCK_PENDING;
		lock->flags |= TTM_SUSPEND_LOCK;
		locked = true;
	} else {
		lock->flags |= TTM_SUSPEND_LOCK_PENDING;
	}
	return locked;
}

void ttm_suspend_lock(struct ttm_lock *lock)
{
	mtx_lock(&lock->lock);
	while (!__ttm_suspend_lock(lock))
		msleep(lock, &lock->lock, 0, "ttms", 0);
	mtx_unlock(&lock->lock);
}