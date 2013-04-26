
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
#include "opt_ktrace.h"
#include "opt_kdtrace.h"

__FBSDID("$FreeBSD$");

#include <sys/capability.h>
#include <sys/ktr.h>
#ifdef KTRACE
#include <sys/uio.h>
#include <sys/ktrace.h>
#endif
#include <security/audit/audit.h>

static inline int
syscallenter(struct thread *td, struct syscall_args *sa)
{
	struct proc *p;
	int error, traced;

	PCPU_INC(cnt.v_syscall);
	p = td->td_proc;

	td->td_pticks = 0;
	if (td->td_ucred != p->p_ucred)
		cred_update_thread(td);
	if (p->p_flag & P_TRACED) {
		traced = 1;
		PROC_LOCK(p);
		td->td_dbgflags &= ~TDB_USERWR;
		td->td_dbgflags |= TDB_SCE;
		PROC_UNLOCK(p);
	} else
		traced = 0;
	error = (p->p_sysent->sv_fetch_syscall_args)(td, sa);
#ifdef KTRACE
	if (KTRPOINT(td, KTR_SYSCALL))
		ktrsyscall(sa->code, sa->narg, sa->args);
#endif
	KTR_START4(KTR_SYSC, "syscall", syscallname(p, sa->code),
	    (uintptr_t)td, "pid:%d", td->td_proc->p_pid, "arg0:%p", sa->args[0],
	    "arg1:%p", sa->args[1], "arg2:%p", sa->args[2]);

	if (error == 0) {

		STOPEVENT(p, S_SCE, sa->narg);
		if (p->p_flag & P_TRACED && p->p_stops & S_PT_SCE) {
			PROC_LOCK(p);
			ptracestop((td), SIGTRAP);
			PROC_UNLOCK(p);
		}
		if (td->td_dbgflags & TDB_USERWR) {
			/*
			 * Reread syscall number and arguments if
			 * debugger modified registers or memory.
			 */
			error = (p->p_sysent->sv_fetch_syscall_args)(td, sa);
#ifdef KTRACE
			if (KTRPOINT(td, KTR_SYSCALL))
				ktrsyscall(sa->code, sa->narg, sa->args);
#endif
			if (error != 0)
				goto retval;
		}

#ifdef CAPABILITY_MODE
		/*
		 * In capability mode, we only allow access to system calls
		 * flagged with SYF_CAPENABLED.
		 */
		if (IN_CAPABILITY_MODE(td) &&
		    !(sa->callp->sy_flags & SYF_CAPENABLED)) {
			error = ECAPMODE;
			goto retval;
		}
#endif

		error = syscall_thread_enter(td, sa->callp);
		if (error != 0)
			goto retval;

#ifdef KDTRACE_HOOKS
		/*
		 * If the systrace module has registered it's probe
		 * callback and if there is a probe active for the
		 * syscall 'entry', process the probe.
		 */
		if (systrace_probe_func != NULL && sa->callp->sy_entry != 0)
			(*systrace_probe_func)(sa->callp->sy_entry, sa->code,
			    sa->callp, sa->args, 0);
#endif

		AUDIT_SYSCALL_ENTER(sa->code, td);
		error = (sa->callp->sy_call)(td, sa->args);
		AUDIT_SYSCALL_EXIT(error, td);

		/* Save the latest error return value. */
		if ((td->td_pflags & TDP_NERRNO) == 0)
			td->td_errno = error;

#ifdef KDTRACE_HOOKS
		/*
		 * If the systrace module has registered it's probe
		 * callback and if there is a probe active for the
		 * syscall 'return', process the probe.
		 */
		if (systrace_probe_func != NULL && sa->callp->sy_return != 0)
			(*systrace_probe_func)(sa->callp->sy_return, sa->code,
			    sa->callp, NULL, (error) ? -1 : td->td_retval[0]);
#endif
		syscall_thread_exit(td, sa->callp);
	}
 retval:
	KTR_STOP4(KTR_SYSC, "syscall", syscallname(p, sa->code),
	    (uintptr_t)td, "pid:%d", td->td_proc->p_pid, "error:%d", error,
	    "retval0:%#lx", td->td_retval[0], "retval1:%#lx",
	    td->td_retval[1]);
	if (traced) {
		PROC_LOCK(p);
		td->td_dbgflags &= ~TDB_SCE;
		PROC_UNLOCK(p);
	}
	(p->p_sysent->sv_set_syscall_retval)(td, error);
	return (error);
}

static inline void
syscallret(struct thread *td, int error, struct syscall_args *sa __unused)
{
	struct proc *p, *p2;
	int traced;

	p = td->td_proc;

	/*
	 * Handle reschedule and other end-of-syscall issues
	 */
	userret(td, td->td_frame);

#ifdef KTRACE
	if (KTRPOINT(td, KTR_SYSRET)) {
		ktrsysret(sa->code, (td->td_pflags & TDP_NERRNO) == 0 ?
		    error : td->td_errno, td->td_retval[0]);
	}
#endif
	td->td_pflags &= ~TDP_NERRNO;

	if (p->p_flag & P_TRACED) {
		traced = 1;
		PROC_LOCK(p);
		td->td_dbgflags |= TDB_SCX;
		PROC_UNLOCK(p);
	} else
		traced = 0;
	/*
	 * This works because errno is findable through the
	 * register set.  If we ever support an emulation where this
	 * is not the case, this code will need to be revisited.
	 */
	STOPEVENT(p, S_SCX, sa->code);
	if (traced || (td->td_dbgflags & (TDB_EXEC | TDB_FORK)) != 0) {
		PROC_LOCK(p);
		/*
		 * If tracing the execed process, trap to the debugger
		 * so that breakpoints can be set before the program
		 * executes.  If debugger requested tracing of syscall
		 * returns, do it now too.
		 */
		if (traced &&
		    ((td->td_dbgflags & (TDB_FORK | TDB_EXEC)) != 0 ||
		    (p->p_stops & S_PT_SCX) != 0))
			ptracestop(td, SIGTRAP);
		td->td_dbgflags &= ~(TDB_SCX | TDB_EXEC | TDB_FORK);
		PROC_UNLOCK(p);
	}

	if (td->td_pflags & TDP_RFPPWAIT) {
		/*
		 * Preserve synchronization semantics of vfork.  If
		 * waiting for child to exec or exit, fork set
		 * P_PPWAIT on child, and there we sleep on our proc
		 * (in case of exit).
		 *
		 * Do it after the ptracestop() above is finished, to
		 * not block our debugger until child execs or exits
		 * to finish vfork wait.
		 */
		td->td_pflags &= ~TDP_RFPPWAIT;
		p2 = td->td_rfppwait_p;
		PROC_LOCK(p2);
		while (p2->p_flag & P_PPWAIT)
			cv_wait(&p2->p_pwait, &p2->p_mtx);
		PROC_UNLOCK(p2);
	}
}