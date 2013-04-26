
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

/* p1003_1b: Real Time common code.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_posix.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/posix4.h>
#include <sys/syscallsubr.h>
#include <sys/sysctl.h>
#include <sys/sysent.h>
#include <sys/syslog.h>
#include <sys/sysproto.h>

MALLOC_DEFINE(M_P31B, "p1003.1b", "Posix 1003.1B");

/* The system calls return ENOSYS if an entry is called that is not run-time
 * supported.  I am also logging since some programs start to use this when
 * they shouldn't.  That will be removed if annoying.
 */
int
syscall_not_present(struct thread *td, const char *s, struct nosys_args *uap)
{
	log(LOG_ERR, "cmd %s pid %d tried to use non-present %s\n",
			td->td_name, td->td_proc->p_pid, s);

	/* a " return nosys(p, uap); " here causes a core dump.
	 */

	return ENOSYS;
}

#if !defined(_KPOSIX_PRIORITY_SCHEDULING)

/* Not configured but loadable via a module:
 */

static int
sched_attach(void)
{
	return 0;
}

SYSCALL_NOT_PRESENT_GEN(sched_setparam)
SYSCALL_NOT_PRESENT_GEN(sched_getparam)
SYSCALL_NOT_PRESENT_GEN(sched_setscheduler)
SYSCALL_NOT_PRESENT_GEN(sched_getscheduler)
SYSCALL_NOT_PRESENT_GEN(sched_yield)
SYSCALL_NOT_PRESENT_GEN(sched_get_priority_max)
SYSCALL_NOT_PRESENT_GEN(sched_get_priority_min)
SYSCALL_NOT_PRESENT_GEN(sched_rr_get_interval)
#else

/* Configured in kernel version:
 */
static struct ksched *ksched;

static int
sched_attach(void)
{
	int ret = ksched_attach(&ksched);

	if (ret == 0)
		p31b_setcfg(CTL_P1003_1B_PRIORITY_SCHEDULING, 200112L);

	return ret;
}

int
sys_sched_setparam(struct thread *td, struct sched_setparam_args *uap)
{
	struct thread *targettd;
	struct proc *targetp;
	int e;
	struct sched_param sched_param;

	e = copyin(uap->param, &sched_param, sizeof(sched_param));
	if (e)
		return (e);

	if (uap->pid == 0) {
		targetp = td->td_proc;
		targettd = td;
		PROC_LOCK(targetp);
	} else {
		targetp = pfind(uap->pid);
		if (targetp == NULL)
			return (ESRCH);
		targettd = FIRST_THREAD_IN_PROC(targetp);
	}

	e = p_cansched(td, targetp);
	if (e == 0) {
		e = ksched_setparam(ksched, targettd,
			(const struct sched_param *)&sched_param);
	}
	PROC_UNLOCK(targetp);
	return (e);
}

int
sys_sched_getparam(struct thread *td, struct sched_getparam_args *uap)
{
	int e;
	struct sched_param sched_param;
	struct thread *targettd;
	struct proc *targetp;

	if (uap->pid == 0) {
		targetp = td->td_proc;
		targettd = td;
		PROC_LOCK(targetp);
	} else {
		targetp = pfind(uap->pid);
		if (targetp == NULL) {
			return (ESRCH);
		}
		targettd = FIRST_THREAD_IN_PROC(targetp);
	}

	e = p_cansee(td, targetp);
	if (e == 0) {
		e = ksched_getparam(ksched, targettd, &sched_param);
	}
	PROC_UNLOCK(targetp);
	if (e == 0)
		e = copyout(&sched_param, uap->param, sizeof(sched_param));
	return (e);
}

int
sys_sched_setscheduler(struct thread *td, struct sched_setscheduler_args *uap)
{
	int e;
	struct sched_param sched_param;
	struct thread *targettd;
	struct proc *targetp;

	/* Don't allow non root user to set a scheduler policy. */
	e = priv_check(td, PRIV_SCHED_SET);
	if (e)
		return (e);

	e = copyin(uap->param, &sched_param, sizeof(sched_param));
	if (e)
		return (e);

	if (uap->pid == 0) {
		targetp = td->td_proc;
		targettd = td;
		PROC_LOCK(targetp);
	} else {
		targetp = pfind(uap->pid);
		if (targetp == NULL)
			return (ESRCH);
		targettd = FIRST_THREAD_IN_PROC(targetp);
	}

	e = p_cansched(td, targetp);
	if (e == 0) {
		e = ksched_setscheduler(ksched, targettd,
			uap->policy, (const struct sched_param *)&sched_param);
	}
	PROC_UNLOCK(targetp);
	return (e);
}

int
sys_sched_getscheduler(struct thread *td, struct sched_getscheduler_args *uap)
{
	int e, policy;
	struct thread *targettd;
	struct proc *targetp;

	if (uap->pid == 0) {
		targetp = td->td_proc;
		targettd = td;
		PROC_LOCK(targetp);
	} else {
		targetp = pfind(uap->pid);
		if (targetp == NULL)
			return (ESRCH);
		targettd = FIRST_THREAD_IN_PROC(targetp);
	}

	e = p_cansee(td, targetp);
	if (e == 0) {
		e = ksched_getscheduler(ksched, targettd, &policy);
		td->td_retval[0] = policy;
	}
	PROC_UNLOCK(targetp);

	return (e);
}

int
sys_sched_yield(struct thread *td, struct sched_yield_args *uap)
{

	sched_relinquish(curthread);
	return 0;
}

int
sys_sched_get_priority_max(struct thread *td,
    struct sched_get_priority_max_args *uap)
{
	int error, prio;

	error = ksched_get_priority_max(ksched, uap->policy, &prio);
	td->td_retval[0] = prio;
	return (error);
}

int
sys_sched_get_priority_min(struct thread *td,
    struct sched_get_priority_min_args *uap)
{
	int error, prio;

	error = ksched_get_priority_min(ksched, uap->policy, &prio);
	td->td_retval[0] = prio;
	return (error);
}

int
sys_sched_rr_get_interval(struct thread *td,
    struct sched_rr_get_interval_args *uap)
{
	struct timespec timespec;
	int error;

	error = kern_sched_rr_get_interval(td, uap->pid, &timespec);
	if (error == 0)
		error = copyout(&timespec, uap->interval, sizeof(timespec));
	return (error);
}

int
kern_sched_rr_get_interval(struct thread *td, pid_t pid,
    struct timespec *ts)
{
	int e;
	struct thread *targettd;
	struct proc *targetp;

	if (pid == 0) {
		targettd = td;
		targetp = td->td_proc;
		PROC_LOCK(targetp);
	} else {
		targetp = pfind(pid);
		if (targetp == NULL)
			return (ESRCH);
		targettd = FIRST_THREAD_IN_PROC(targetp);
	}

	e = p_cansee(td, targetp);
	if (e == 0)
		e = ksched_rr_get_interval(ksched, targettd, ts);
	PROC_UNLOCK(targetp);
	return (e);
}

#endif

static void
p31binit(void *notused)
{
	(void) sched_attach();
	p31b_setcfg(CTL_P1003_1B_PAGESIZE, PAGE_SIZE);
}

SYSINIT(p31b, SI_SUB_P1003_1B, SI_ORDER_FIRST, p31binit, NULL);