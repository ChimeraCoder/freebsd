
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

#include "opt_compat.h"
#include "opt_kdtrace.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/imgact.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/sdt.h>
#include <sys/sx.h>
#include <sys/unistd.h>

#ifdef COMPAT_LINUX32
#include <machine/../linux32/linux.h>
#include <machine/../linux32/linux32_proto.h>
#else
#include <machine/../linux/linux.h>
#include <machine/../linux/linux_proto.h>
#endif
#include <compat/linux/linux_dtrace.h>
#include <compat/linux/linux_signal.h>
#include <compat/linux/linux_emul.h>
#include <compat/linux/linux_misc.h>

/* DTrace init */
LIN_SDT_PROVIDER_DECLARE(LINUX_DTRACE);

/* Linuxulator-global DTrace probes */
LIN_SDT_PROBE_DECLARE(locks, emul_lock, locked);
LIN_SDT_PROBE_DECLARE(locks, emul_lock, unlock);


int
linux_fork(struct thread *td, struct linux_fork_args *args)
{
	int error;
	struct proc *p2;
	struct thread *td2;

#ifdef DEBUG
	if (ldebug(fork))
		printf(ARGS(fork, ""));
#endif

	if ((error = fork1(td, RFFDG | RFPROC | RFSTOPPED, 0, &p2, NULL, 0))
	    != 0)
		return (error);

	td->td_retval[0] = p2->p_pid;
	td->td_retval[1] = 0;

	error = linux_proc_init(td, td->td_retval[0], 0);
	if (error)
		return (error);

	td2 = FIRST_THREAD_IN_PROC(p2);

	/*
	 * Make this runnable after we are finished with it.
	 */
	thread_lock(td2);
	TD_SET_CAN_RUN(td2);
	sched_add(td2, SRQ_BORING);
	thread_unlock(td2);

	return (0);
}

int
linux_vfork(struct thread *td, struct linux_vfork_args *args)
{
	int error;
	struct proc *p2;
	struct thread *td2;

#ifdef DEBUG
	if (ldebug(vfork))
		printf(ARGS(vfork, ""));
#endif

	/* Exclude RFPPWAIT */
	if ((error = fork1(td, RFFDG | RFPROC | RFMEM | RFSTOPPED, 0, &p2,
	    NULL, 0)) != 0)
		return (error);

   	td->td_retval[0] = p2->p_pid;

	error = linux_proc_init(td, td->td_retval[0], 0);
	if (error)
		return (error);

	PROC_LOCK(p2);
	p2->p_flag |= P_PPWAIT;
	PROC_UNLOCK(p2);

	td2 = FIRST_THREAD_IN_PROC(p2);

	/*
	 * Make this runnable after we are finished with it.
	 */
	thread_lock(td2);
	TD_SET_CAN_RUN(td2);
	sched_add(td2, SRQ_BORING);
	thread_unlock(td2);

	/* wait for the children to exit, ie. emulate vfork */
	PROC_LOCK(p2);
	while (p2->p_flag & P_PPWAIT)
		cv_wait(&p2->p_pwait, &p2->p_mtx);
	PROC_UNLOCK(p2);

	return (0);
}

int
linux_clone(struct thread *td, struct linux_clone_args *args)
{
	int error, ff = RFPROC | RFSTOPPED;
	struct proc *p2;
	struct thread *td2;
	int exit_signal;
	struct linux_emuldata *em;

#ifdef DEBUG
	if (ldebug(clone)) {
		printf(ARGS(clone, "flags %x, stack %p, parent tid: %p, "
		    "child tid: %p"), (unsigned)args->flags,
		    args->stack, args->parent_tidptr, args->child_tidptr);
	}
#endif

	exit_signal = args->flags & 0x000000ff;
	if (LINUX_SIG_VALID(exit_signal)) {
		if (exit_signal <= LINUX_SIGTBLSZ)
			exit_signal =
			    linux_to_bsd_signal[_SIG_IDX(exit_signal)];
	} else if (exit_signal != 0)
		return (EINVAL);

	if (args->flags & LINUX_CLONE_VM)
		ff |= RFMEM;
	if (args->flags & LINUX_CLONE_SIGHAND)
		ff |= RFSIGSHARE;
	/*
	 * XXX: In Linux, sharing of fs info (chroot/cwd/umask)
	 * and open files is independant.  In FreeBSD, its in one
	 * structure but in reality it does not cause any problems
	 * because both of these flags are usually set together.
	 */
	if (!(args->flags & (LINUX_CLONE_FILES | LINUX_CLONE_FS)))
		ff |= RFFDG;

	/*
	 * Attempt to detect when linux_clone(2) is used for creating
	 * kernel threads. Unfortunately despite the existence of the
	 * CLONE_THREAD flag, version of linuxthreads package used in
	 * most popular distros as of beginning of 2005 doesn't make
	 * any use of it. Therefore, this detection relies on
	 * empirical observation that linuxthreads sets certain
	 * combination of flags, so that we can make more or less
	 * precise detection and notify the FreeBSD kernel that several
	 * processes are in fact part of the same threading group, so
	 * that special treatment is necessary for signal delivery
	 * between those processes and fd locking.
	 */
	if ((args->flags & 0xffffff00) == LINUX_THREADING_FLAGS)
		ff |= RFTHREAD;

	if (args->flags & LINUX_CLONE_PARENT_SETTID)
		if (args->parent_tidptr == NULL)
			return (EINVAL);

	error = fork1(td, ff, 0, &p2, NULL, 0);
	if (error)
		return (error);

	if (args->flags & (LINUX_CLONE_PARENT | LINUX_CLONE_THREAD)) {
	   	sx_xlock(&proctree_lock);
		PROC_LOCK(p2);
		proc_reparent(p2, td->td_proc->p_pptr);
		PROC_UNLOCK(p2);
		sx_xunlock(&proctree_lock);
	}

	/* create the emuldata */
	error = linux_proc_init(td, p2->p_pid, args->flags);
	/* reference it - no need to check this */
	em = em_find(p2, EMUL_DOLOCK);
	KASSERT(em != NULL, ("clone: emuldata not found."));
	/* and adjust it */

	if (args->flags & LINUX_CLONE_THREAD) {
#ifdef notyet
	   	PROC_LOCK(p2);
	   	p2->p_pgrp = td->td_proc->p_pgrp;
	   	PROC_UNLOCK(p2);
#endif
		exit_signal = 0;
	}

	if (args->flags & LINUX_CLONE_CHILD_SETTID)
		em->child_set_tid = args->child_tidptr;
	else
	   	em->child_set_tid = NULL;

	if (args->flags & LINUX_CLONE_CHILD_CLEARTID)
		em->child_clear_tid = args->child_tidptr;
	else
	   	em->child_clear_tid = NULL;

	EMUL_UNLOCK(&emul_lock);

	if (args->flags & LINUX_CLONE_PARENT_SETTID) {
		error = copyout(&p2->p_pid, args->parent_tidptr,
		    sizeof(p2->p_pid));
		if (error)
			printf(LMSG("copyout failed!"));
	}

	PROC_LOCK(p2);
	p2->p_sigparent = exit_signal;
	PROC_UNLOCK(p2);
	td2 = FIRST_THREAD_IN_PROC(p2);
	/*
	 * In a case of stack = NULL, we are supposed to COW calling process
	 * stack. This is what normal fork() does, so we just keep tf_rsp arg
	 * intact.
	 */
	if (args->stack)
		linux_set_upcall_kse(td2, PTROUT(args->stack));

	if (args->flags & LINUX_CLONE_SETTLS)
		linux_set_cloned_tls(td2, args->tls);

#ifdef DEBUG
	if (ldebug(clone))
		printf(LMSG("clone: successful rfork to %d, "
		    "stack %p sig = %d"), (int)p2->p_pid, args->stack,
		    exit_signal);
#endif
	if (args->flags & LINUX_CLONE_VFORK) {
	   	PROC_LOCK(p2);
	   	p2->p_flag |= P_PPWAIT;
	   	PROC_UNLOCK(p2);
	}

	/*
	 * Make this runnable after we are finished with it.
	 */
	thread_lock(td2);
	TD_SET_CAN_RUN(td2);
	sched_add(td2, SRQ_BORING);
	thread_unlock(td2);

	td->td_retval[0] = p2->p_pid;
	td->td_retval[1] = 0;

	if (args->flags & LINUX_CLONE_VFORK) {
		/* wait for the children to exit, ie. emulate vfork */
		PROC_LOCK(p2);
		while (p2->p_flag & P_PPWAIT)
			cv_wait(&p2->p_pwait, &p2->p_mtx);
		PROC_UNLOCK(p2);
	}

	return (0);
}