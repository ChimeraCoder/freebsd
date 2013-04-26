
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

#include "opt_capsicum.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/capability.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/sysproto.h>

#include <machine/md_var.h>
#include <machine/utrap.h>
#include <machine/sysarch.h>

static int sparc_sigtramp_install(struct thread *td, char *args);
static int sparc_utrap_install(struct thread *td, char *args);

#ifndef	_SYS_SYSPROTO_H_
struct sysarch_args {
	int	op;
	char	*parms;
};
#endif

int
sysarch(struct thread *td, struct sysarch_args *uap)
{
	int error;

#ifdef CAPABILITY_MODE
	/*
	 * When adding new operations, add a new case statement here to
	 * explicitly indicate whether or not the operation is safe to
	 * perform in capability mode.
	 */
	if (IN_CAPABILITY_MODE(td)) {
		switch (uap->op) {
		case SPARC_SIGTRAMP_INSTALL:
		case SPARC_UTRAP_INSTALL:
			break;

		default:
#ifdef KTRACE
			if (KTRPOINT(td, KTR_CAPFAIL))
				ktrcapfail(CAPFAIL_SYSCALL, 0, 0);
#endif
			return (ECAPMODE);
		}
	}
#endif

	mtx_lock(&Giant);
	switch (uap->op) {
	case SPARC_SIGTRAMP_INSTALL:
		error = sparc_sigtramp_install(td, uap->parms);
		break;
	case SPARC_UTRAP_INSTALL:
		error = sparc_utrap_install(td, uap->parms);
		break;
	default:
		error = EINVAL;
		break;
	}
	mtx_unlock(&Giant);
	return (error);
}

static int
sparc_sigtramp_install(struct thread *td, char *args)
{
	struct sparc_sigtramp_install_args sia;
	struct proc *p;
	int error;

	p = td->td_proc;
	if ((error = copyin(args, &sia, sizeof(sia))) != 0)
		return (error);
	if (sia.sia_old != NULL) {
		if (suword(sia.sia_old, (long)p->p_md.md_sigtramp) != 0)
			return (EFAULT);
	}
	p->p_md.md_sigtramp = sia.sia_new;
	return (0);
}

static int
sparc_utrap_install(struct thread *td, char *args)
{
	struct sparc_utrap_install_args uia;
	struct sparc_utrap_args ua;
	struct md_utrap *ut;
	int error;
	int i;

	ut = td->td_proc->p_md.md_utrap;
	if ((error = copyin(args, &uia, sizeof(uia))) != 0)
		return (error);
	if (uia.num < 0 || uia.num > UT_MAX ||
	    (uia.handlers == NULL && uia.num > 0))
		return (EINVAL);
	for (i = 0; i < uia.num; i++) {
		if ((error = copyin(&uia.handlers[i], &ua, sizeof(ua))) != 0)
			return (error);
		if (ua.type != UTH_NOCHANGE &&
		    (ua.type < 0 || ua.type >= UT_MAX))
			return (EINVAL);
		if (ua.old_deferred != NULL) {
			if ((error = suword(ua.old_deferred, 0)) != 0)
				return (error);
		}
		if (ua.old_precise != NULL) {
			error = suword(ua.old_precise,
			    ut != NULL ? (long)ut->ut_precise[ua.type] : 0);
			if (error != 0)
				return (error);
		}
		if (ua.type != UTH_NOCHANGE) {
			if (ut == NULL) {
				ut = utrap_alloc();
				td->td_proc->p_md.md_utrap = ut;
			}
			ut->ut_precise[ua.type] = ua.new_precise;
		}
	}
	return (0);
}