
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
 * Processes may set login class name using setloginclass(2).  This
 * is usually done through call to setusercontext(3), by programs
 * such as login(1), based on information from master.passwd(5).  Kernel
 * uses this information to enforce per-class resource limits.  Current
 * login class can be determined using id(1).  Login class is inherited
 * from the parent process during fork(2).  If not set, it defaults
 * to "default".
 *
 * Code in this file implements setloginclass(2) and getloginclass(2)
 * system calls, and maintains class name storage and retrieval.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/eventhandler.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/loginclass.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/types.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/racct.h>
#include <sys/refcount.h>
#include <sys/sysproto.h>
#include <sys/systm.h>

static MALLOC_DEFINE(M_LOGINCLASS, "loginclass", "loginclass structures");

LIST_HEAD(, loginclass)	loginclasses;

/*
 * Lock protecting loginclasses list.
 */
static struct mtx loginclasses_lock;

static void lc_init(void);
SYSINIT(loginclass, SI_SUB_CPU, SI_ORDER_FIRST, lc_init, NULL);

void
loginclass_hold(struct loginclass *lc)
{

	refcount_acquire(&lc->lc_refcount);
}

void
loginclass_free(struct loginclass *lc)
{
	int old;

	old = lc->lc_refcount;
	if (old > 1 && atomic_cmpset_int(&lc->lc_refcount, old, old - 1))
		return;

	mtx_lock(&loginclasses_lock);
	if (refcount_release(&lc->lc_refcount)) {
		racct_destroy(&lc->lc_racct);
		LIST_REMOVE(lc, lc_next);
		mtx_unlock(&loginclasses_lock);
		free(lc, M_LOGINCLASS);

		return;
	}
	mtx_unlock(&loginclasses_lock);
}

/*
 * Return loginclass structure with a corresponding name.  Not
 * performance critical, as it's used mainly by setloginclass(2),
 * which happens once per login session.  Caller has to use
 * loginclass_free() on the returned value when it's no longer
 * needed.
 */
struct loginclass *
loginclass_find(const char *name)
{
	struct loginclass *lc, *newlc;

	if (name[0] == '\0' || strlen(name) >= MAXLOGNAME)
		return (NULL);

	newlc = malloc(sizeof(*newlc), M_LOGINCLASS, M_ZERO | M_WAITOK);
	racct_create(&newlc->lc_racct);

	mtx_lock(&loginclasses_lock);
	LIST_FOREACH(lc, &loginclasses, lc_next) {
		if (strcmp(name, lc->lc_name) != 0)
			continue;

		/* Found loginclass with a matching name? */
		loginclass_hold(lc);
		mtx_unlock(&loginclasses_lock);
		racct_destroy(&newlc->lc_racct);
		free(newlc, M_LOGINCLASS);
		return (lc);
	}

	/* Add new loginclass. */
	strcpy(newlc->lc_name, name);
	refcount_init(&newlc->lc_refcount, 1);
	LIST_INSERT_HEAD(&loginclasses, newlc, lc_next);
	mtx_unlock(&loginclasses_lock);

	return (newlc);
}

/*
 * Get login class name.
 */
#ifndef _SYS_SYSPROTO_H_
struct getloginclass_args {
	char	*namebuf;
	size_t	namelen;
};
#endif
/* ARGSUSED */
int
sys_getloginclass(struct thread *td, struct getloginclass_args *uap)
{
	int error = 0;
	size_t lcnamelen;
	struct proc *p;
	struct loginclass *lc;

	p = td->td_proc;
	PROC_LOCK(p);
	lc = p->p_ucred->cr_loginclass;
	loginclass_hold(lc);
	PROC_UNLOCK(p);

	lcnamelen = strlen(lc->lc_name) + 1;
	if (lcnamelen > uap->namelen)
		error = ERANGE;
	if (error == 0)
		error = copyout(lc->lc_name, uap->namebuf, lcnamelen);
	loginclass_free(lc);
	return (error);
}

/*
 * Set login class name.
 */
#ifndef _SYS_SYSPROTO_H_
struct setloginclass_args {
	const char	*namebuf;
};
#endif
/* ARGSUSED */
int
sys_setloginclass(struct thread *td, struct setloginclass_args *uap)
{
	struct proc *p = td->td_proc;
	int error;
	char lcname[MAXLOGNAME];
	struct loginclass *newlc;
	struct ucred *newcred, *oldcred;

	error = priv_check(td, PRIV_PROC_SETLOGINCLASS);
	if (error != 0)
		return (error);
	error = copyinstr(uap->namebuf, lcname, sizeof(lcname), NULL);
	if (error != 0)
		return (error);

	newlc = loginclass_find(lcname);
	if (newlc == NULL)
		return (EINVAL);
	newcred = crget();

	PROC_LOCK(p);
	oldcred = crcopysafe(p, newcred);
	newcred->cr_loginclass = newlc;
	p->p_ucred = newcred;
	PROC_UNLOCK(p);
#ifdef RACCT
	racct_proc_ucred_changed(p, oldcred, newcred);
#endif
	loginclass_free(oldcred->cr_loginclass);
	crfree(oldcred);

	return (0);
}

void
loginclass_racct_foreach(void (*callback)(struct racct *racct,
    void *arg2, void *arg3), void *arg2, void *arg3)
{
	struct loginclass *lc;

	mtx_lock(&loginclasses_lock);
	LIST_FOREACH(lc, &loginclasses, lc_next)
		(callback)(lc->lc_racct, arg2, arg3);
	mtx_unlock(&loginclasses_lock);
}

static void
lc_init(void)
{

	mtx_init(&loginclasses_lock, "loginclasses lock", NULL, MTX_DEF);
}