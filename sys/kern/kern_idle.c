
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
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/resourcevar.h>
#include <sys/sched.h>
#include <sys/unistd.h>
#ifdef SMP
#include <sys/smp.h>
#endif

static void idle_setup(void *dummy);
SYSINIT(idle_setup, SI_SUB_SCHED_IDLE, SI_ORDER_FIRST, idle_setup, NULL);

/*
 * Set up per-cpu idle process contexts.  The AP's shouldn't be running or
 * accessing their idle processes at this point, so don't bother with
 * locking.
 */
static void
idle_setup(void *dummy)
{
#ifdef SMP
	struct pcpu *pc;
#endif
	struct proc *p;
	struct thread *td;
	int error;

	p = NULL; /* start with no idle process */
#ifdef SMP
	STAILQ_FOREACH(pc, &cpuhead, pc_allcpu) {
#endif
#ifdef SMP
		error = kproc_kthread_add(sched_idletd, NULL, &p, &td,
		    RFSTOPPED | RFHIGHPID, 0, "idle", "idle: cpu%d", pc->pc_cpuid);
		pc->pc_idlethread = td;
#else
		error = kproc_kthread_add(sched_idletd, NULL, &p, &td,
		    RFSTOPPED | RFHIGHPID, 0, "idle", "idle");
		PCPU_SET(idlethread, td);
#endif
		if (error)
			panic("idle_setup: kproc_create error %d\n", error);

		thread_lock(td);
		TD_SET_CAN_RUN(td);
		td->td_flags |= TDF_IDLETD | TDF_NOLOAD;
		sched_class(td, PRI_IDLE);
		sched_prio(td, PRI_MAX_IDLE);
		thread_unlock(td);
#ifdef SMP
	}
#endif
}