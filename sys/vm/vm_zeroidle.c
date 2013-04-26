
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

#include <opt_sched.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/vmmeter.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/sched.h>
#include <sys/sysctl.h>
#include <sys/kthread.h>
#include <sys/unistd.h>

#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_phys.h>

static int idlezero_enable_default = 0;
TUNABLE_INT("vm.idlezero_enable", &idlezero_enable_default);
/* Defer setting the enable flag until the kthread is running. */
static int idlezero_enable = 0;
SYSCTL_INT(_vm, OID_AUTO, idlezero_enable, CTLFLAG_RW, &idlezero_enable, 0,
    "Allow the kernel to use idle cpu cycles to zero-out pages");
/*
 * Implement the pre-zeroed page mechanism.
 */

#define ZIDLE_LO(v)	((v) * 2 / 3)
#define ZIDLE_HI(v)	((v) * 4 / 5)

static boolean_t wakeup_needed = FALSE;
static int zero_state;

static int
vm_page_zero_check(void)
{

	if (!idlezero_enable)
		return (0);
	/*
	 * Attempt to maintain approximately 1/2 of our free pages in a
	 * PG_ZERO'd state.   Add some hysteresis to (attempt to) avoid
	 * generally zeroing a page when the system is near steady-state.
	 * Otherwise we might get 'flutter' during disk I/O / IPC or 
	 * fast sleeps.  We also do not want to be continuously zeroing
	 * pages because doing so may flush our L1 and L2 caches too much.
	 */
	if (zero_state && vm_page_zero_count >= ZIDLE_LO(cnt.v_free_count))
		return (0);
	if (vm_page_zero_count >= ZIDLE_HI(cnt.v_free_count))
		return (0);
	return (1);
}

static void
vm_page_zero_idle(void)
{

	mtx_assert(&vm_page_queue_free_mtx, MA_OWNED);
	zero_state = 0;
	if (vm_phys_zero_pages_idle()) {
		if (vm_page_zero_count >= ZIDLE_HI(cnt.v_free_count))
			zero_state = 1;
	}
}

/* Called by vm_page_free to hint that a new page is available. */
void
vm_page_zero_idle_wakeup(void)
{

	mtx_assert(&vm_page_queue_free_mtx, MA_OWNED);
	if (wakeup_needed && vm_page_zero_check()) {
		wakeup_needed = FALSE;
		wakeup(&zero_state);
	}
}

static void
vm_pagezero(void __unused *arg)
{

	idlezero_enable = idlezero_enable_default;

	mtx_lock(&vm_page_queue_free_mtx);
	for (;;) {
		if (vm_page_zero_check()) {
			vm_page_zero_idle();
#ifndef PREEMPTION
			if (sched_runnable()) {
				thread_lock(curthread);
				mi_switch(SW_VOL | SWT_IDLE, NULL);
				thread_unlock(curthread);
			}
#endif
		} else {
			wakeup_needed = TRUE;
			msleep(&zero_state, &vm_page_queue_free_mtx, 0,
			    "pgzero", hz * 300);
		}
	}
}

static void
pagezero_start(void __unused *arg)
{
	int error;
	struct proc *p;
	struct thread *td;

	error = kproc_create(vm_pagezero, NULL, &p, RFSTOPPED, 0, "pagezero");
	if (error)
		panic("pagezero_start: error %d\n", error);
	td = FIRST_THREAD_IN_PROC(p);
	thread_lock(td);

	/* We're an idle task, don't count us in the load. */
	td->td_flags |= TDF_NOLOAD;
	sched_class(td, PRI_IDLE);
	sched_prio(td, PRI_MAX_IDLE);
	sched_add(td, SRQ_BORING);
	thread_unlock(td);
}
SYSINIT(pagezero, SI_SUB_KTHREAD_VM, SI_ORDER_ANY, pagezero_start, NULL);