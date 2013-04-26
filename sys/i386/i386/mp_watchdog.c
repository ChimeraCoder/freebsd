
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

#include "opt_mp_watchdog.h"
#include "opt_sched.h"

#ifdef SCHED_ULE
#error MP_WATCHDOG cannot currently be used with SCHED_ULE
#endif

#include <sys/param.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <machine/smp.h>
#include <x86/apicreg.h>
#include <machine/apicvar.h>
#include <machine/mp_watchdog.h>

/*
 * mp_watchdog hijacks the idle thread on a specified CPU, prevents new work
 * from being scheduled there, and uses it as a "watchdog" to detect kernel
 * failure on other CPUs.  This is made reasonable by inclusion of logical
 * processors in Xeon hardware.  The watchdog is configured by setting the
 * debug.watchdog sysctl/tunable to the CPU of interest.  A callout will then
 * begin executing reseting a timer that is gradually lowered by the watching
 * thread.  If the timer reaches 0, the watchdog fires by ether dropping
 * directly to the debugger, or by sending an NMI IPI to the boot processor.
 * This is a somewhat less efficient substitute for dedicated watchdog
 * hardware, but can be quite an effective tool for debugging hangs.
 *
 * XXXRW: This should really use the watchdog(9)/watchdog(4) framework, but
 * doesn't yet.
 */
static int	watchdog_cpu = -1;
static int	watchdog_dontfire = 1;
static int	watchdog_timer = -1;
static int	watchdog_nmi = 1;

TUNABLE_INT("debug.watchdog", &watchdog_cpu);
SYSCTL_INT(_debug, OID_AUTO, watchdog_nmi, CTLFLAG_RW, &watchdog_nmi, 0,
    "IPI the boot processor with an NMI to enter the debugger");

static struct callout	watchdog_callout;

static void watchdog_change(int wdcpu);

/*
 * Number of seconds before the watchdog will fire if the callout fails to
 * reset the timer.
 */
#define	WATCHDOG_THRESHOLD	10

static void
watchdog_init(void *arg)
{

	callout_init(&watchdog_callout, CALLOUT_MPSAFE);
	if (watchdog_cpu != -1)
		watchdog_change(watchdog_cpu);
}

/*
 * This callout resets a timer until the watchdog kicks in.  It acquires some
 * critical locks to make sure things haven't gotten wedged with hose locks
 * held.
 */
static void
watchdog_function(void *arg)
{

	/*
	 * Since the timer ran, we must not be wedged.  Acquire some critical
	 * locks to make sure.  Then reset the timer.
	 */
	mtx_lock(&Giant);
	watchdog_timer = WATCHDOG_THRESHOLD;
	mtx_unlock(&Giant);
	callout_reset(&watchdog_callout, 1 * hz, watchdog_function, NULL);
}
SYSINIT(watchdog_init, SI_SUB_DRIVERS, SI_ORDER_ANY, watchdog_init, NULL);

static void
watchdog_change(int wdcpu)
{

	if (wdcpu == -1 || wdcpu == 0xffffffff) {
		/*
		 * Disable the watchdog.
		 */
		watchdog_cpu = -1;
		watchdog_dontfire = 1;
		callout_stop(&watchdog_callout);
		printf("watchdog stopped\n");
	} else {
		watchdog_timer = WATCHDOG_THRESHOLD;
		watchdog_dontfire = 0;
		watchdog_cpu = wdcpu;
		callout_reset(&watchdog_callout, 1 * hz, watchdog_function,
		    NULL);
	}
}

/*
 * This sysctl sets which CPU is the watchdog CPU.  Set to -1 or 0xffffffff
 * to disable the watchdog.
 */
static int
sysctl_watchdog(SYSCTL_HANDLER_ARGS)
{
	int error, temp;

	temp = watchdog_cpu;
	error = sysctl_handle_int(oidp, &temp, 0, req);
	if (error)
		return (error);

	if (req->newptr != NULL)
		watchdog_change(temp);
	return (0);
}
SYSCTL_PROC(_debug, OID_AUTO, watchdog, CTLTYPE_INT|CTLFLAG_RW, 0, 0,
    sysctl_watchdog, "I", "");

/*
 * Drop into the debugger by sending an IPI NMI to the boot processor.
 */
static void
watchdog_ipi_nmi(void)
{

	/*
	 * Deliver NMI to the boot processor.  Why not?
	 */
	lapic_ipi_raw(APIC_DEST_DESTFLD | APIC_TRIGMOD_EDGE |
	    APIC_LEVEL_ASSERT | APIC_DESTMODE_PHY | APIC_DELMODE_NMI,
	    boot_cpu_id);
	lapic_ipi_wait(-1);
}

/*
 * ap_watchdog() is called by the SMP idle loop code.  It works on the same
 * premise that the disabling of logical processors does: that if the cpu is
 * idle, then it can ignore the world from then on, as nothing will be
 * scheduled on it.  Leaving aside multi-runqueue schedulers (SCHED_ULE) and
 * explicit process migration (sched_bind()), this is not an unreasonable
 * assumption.
 */
void
ap_watchdog(u_int cpuid)
{
	char old_pcomm[MAXCOMLEN + 1];
	struct proc *p;

	if (watchdog_cpu != cpuid)
		return;

	printf("watchdog started on cpu %d\n", cpuid);
	p = curproc;
	bcopy(p->p_comm, old_pcomm, MAXCOMLEN + 1);
	snprintf(p->p_comm, MAXCOMLEN + 1, "mp_watchdog cpu %d", cpuid);
	while (1) {
		DELAY(1000000);				/* One second. */
		if (watchdog_cpu != cpuid)
			break;
		atomic_subtract_int(&watchdog_timer, 1);
		if (watchdog_timer < 4)
			printf("Watchdog timer: %d\n", watchdog_timer);
		if (watchdog_timer == 0 && watchdog_dontfire == 0) {
			printf("Watchdog firing!\n");
			watchdog_dontfire = 1;
			if (watchdog_nmi)
				watchdog_ipi_nmi();
			else
				kdb_enter(KDB_WHY_WATCHDOG, "mp_watchdog");
		}
	}
	bcopy(old_pcomm, p->p_comm, MAXCOMLEN + 1);
	printf("watchdog stopped on cpu %d\n", cpuid);
}