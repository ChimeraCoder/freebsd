
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
#include <sys/cpuset.h>
#include <sys/ktr.h>
#include <sys/proc.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/kernel.h>
#include <sys/pcpu.h>
#include <sys/smp.h>
#include <sys/sched.h>
#include <sys/bus.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>
#include <vm/vm_kern.h>

#include <machine/clock.h>
#include <machine/smp.h>
#include <machine/hwfunc.h>
#include <machine/intr_machdep.h>
#include <machine/cache.h>
#include <machine/tlb.h>

struct pcb stoppcbs[MAXCPU];

static void *dpcpu;
static struct mtx ap_boot_mtx;

static volatile int aps_ready;
static volatile int mp_naps;

static void
ipi_send(struct pcpu *pc, int ipi)
{

	CTR3(KTR_SMP, "%s: cpu=%d, ipi=%x", __func__, pc->pc_cpuid, ipi);

	atomic_set_32(&pc->pc_pending_ipis, ipi);
	platform_ipi_send(pc->pc_cpuid);

	CTR1(KTR_SMP, "%s: sent", __func__);
}

void
ipi_all_but_self(int ipi)
{
	cpuset_t other_cpus;

	other_cpus = all_cpus;
	CPU_CLR(PCPU_GET(cpuid), &other_cpus);
	ipi_selected(other_cpus, ipi);
}

/* Send an IPI to a set of cpus. */
void
ipi_selected(cpuset_t cpus, int ipi)
{
	struct pcpu *pc;

	STAILQ_FOREACH(pc, &cpuhead, pc_allcpu) {
		if (CPU_ISSET(pc->pc_cpuid, &cpus)) {
			CTR3(KTR_SMP, "%s: pc: %p, ipi: %x\n", __func__, pc,
			    ipi);
			ipi_send(pc, ipi);
		}
	}
}

/* Send an IPI to a specific CPU. */
void
ipi_cpu(int cpu, u_int ipi)
{

	CTR3(KTR_SMP, "%s: cpu: %d, ipi: %x\n", __func__, cpu, ipi);
	ipi_send(cpuid_to_pcpu[cpu], ipi);
}

/*
 * Handle an IPI sent to this processor.
 */
static int
mips_ipi_handler(void *arg)
{
	u_int	cpu, ipi, ipi_bitmap;
	int	bit;

	cpu = PCPU_GET(cpuid);

	platform_ipi_clear();	/* quiesce the pending ipi interrupt */

	ipi_bitmap = atomic_readandclear_int(PCPU_PTR(pending_ipis));
	if (ipi_bitmap == 0)
		return (FILTER_STRAY);

	CTR1(KTR_SMP, "smp_handle_ipi(), ipi_bitmap=%x", ipi_bitmap);

	while ((bit = ffs(ipi_bitmap))) {
		bit = bit - 1;
		ipi = 1 << bit;
		ipi_bitmap &= ~ipi;
		switch (ipi) {
		case IPI_RENDEZVOUS:
			CTR0(KTR_SMP, "IPI_RENDEZVOUS");
			smp_rendezvous_action();
			break;

		case IPI_AST:
			CTR0(KTR_SMP, "IPI_AST");
			break;

		case IPI_STOP:
			/*
			 * IPI_STOP_HARD is mapped to IPI_STOP so it is not
			 * necessary to add it in the switch.
			 */
			CTR0(KTR_SMP, "IPI_STOP or IPI_STOP_HARD");

			savectx(&stoppcbs[cpu]);
			tlb_save();

			/* Indicate we are stopped */
			CPU_SET_ATOMIC(cpu, &stopped_cpus);

			/* Wait for restart */
			while (!CPU_ISSET(cpu, &started_cpus))
				cpu_spinwait();

			CPU_CLR_ATOMIC(cpu, &started_cpus);
			CPU_CLR_ATOMIC(cpu, &stopped_cpus);
			CTR0(KTR_SMP, "IPI_STOP (restart)");
			break;
		case IPI_PREEMPT:
			CTR1(KTR_SMP, "%s: IPI_PREEMPT", __func__);
			sched_preempt(curthread);
			break;
		case IPI_HARDCLOCK:
			CTR1(KTR_SMP, "%s: IPI_HARDCLOCK", __func__);
			hardclockintr();
			break;
		default:
			panic("Unknown IPI 0x%0x on cpu %d", ipi, curcpu);
		}
	}

	return (FILTER_HANDLED);
}

static int
start_ap(int cpuid)
{
	int cpus, ms;

	cpus = mp_naps;
	dpcpu = (void *)kmem_alloc(kernel_map, DPCPU_SIZE);

	mips_sync();

	if (platform_start_ap(cpuid) != 0)
		return (-1);			/* could not start AP */

	for (ms = 0; ms < 5000; ++ms) {
		if (mp_naps > cpus)
			return (0);		/* success */
		else
			DELAY(1000);
	}

	return (-2);				/* timeout initializing AP */
}

void
cpu_mp_setmaxid(void)
{
	cpuset_t cpumask;
	int cpu, last;

	platform_cpu_mask(&cpumask);
	mp_ncpus = 0;
	last = 1;
	while ((cpu = cpusetobj_ffs(&cpumask)) != 0) {
		last = cpu;
		cpu--;
		CPU_CLR(cpu, &cpumask);
		mp_ncpus++;
	}
	if (mp_ncpus <= 0)
		mp_ncpus = 1;

	mp_maxid = min(last, MAXCPU) - 1;
}

void
cpu_mp_announce(void)
{
	/* NOTHING */
}

struct cpu_group *
cpu_topo(void)
{
	return (platform_smp_topo());
}

int
cpu_mp_probe(void)
{

	return (mp_ncpus > 1);
}

void
cpu_mp_start(void)
{
	int error, cpuid;
	cpuset_t cpumask;

	mtx_init(&ap_boot_mtx, "ap boot", NULL, MTX_SPIN);

	CPU_ZERO(&all_cpus);
	platform_cpu_mask(&cpumask);

	while (!CPU_EMPTY(&cpumask)) {
		cpuid = cpusetobj_ffs(&cpumask) - 1;
		CPU_CLR(cpuid, &cpumask);

		if (cpuid >= MAXCPU) {
			printf("cpu_mp_start: ignoring AP #%d.\n", cpuid);
			continue;
		}

		if (cpuid != platform_processor_id()) {
			if ((error = start_ap(cpuid)) != 0) {
				printf("AP #%d failed to start: %d\n", cpuid, error);
				continue;
			}
			if (bootverbose)
				printf("AP #%d started!\n", cpuid);
		}
		CPU_SET(cpuid, &all_cpus);
	}
}

void
smp_init_secondary(u_int32_t cpuid)
{

	/* TLB */
	mips_wr_wired(0);
	tlb_invalidate_all();
	mips_wr_wired(VMWIRED_ENTRIES);

	/*
	 * We assume that the L1 cache on the APs is identical to the one
	 * on the BSP.
	 */
	mips_dcache_wbinv_all();
	mips_icache_sync_all();

	mips_sync();

	mips_wr_entryhi(0);

	pcpu_init(PCPU_ADDR(cpuid), cpuid, sizeof(struct pcpu));
	dpcpu_init(dpcpu, cpuid);

	/* The AP has initialized successfully - allow the BSP to proceed */
	++mp_naps;

	/* Spin until the BSP is ready to release the APs */
	while (!aps_ready)
		;

	/* Initialize curthread. */
	KASSERT(PCPU_GET(idlethread) != NULL, ("no idle thread"));
	PCPU_SET(curthread, PCPU_GET(idlethread));

	mtx_lock_spin(&ap_boot_mtx);

	smp_cpus++;

	CTR1(KTR_SMP, "SMP: AP CPU #%d launched", PCPU_GET(cpuid));

	if (bootverbose)
		printf("SMP: AP CPU #%d launched.\n", PCPU_GET(cpuid));

	if (smp_cpus == mp_ncpus) {
		atomic_store_rel_int(&smp_started, 1);
		smp_active = 1;
	}

	mtx_unlock_spin(&ap_boot_mtx);

	while (smp_started == 0)
		; /* nothing */

	/* Start per-CPU event timers. */
	cpu_initclocks_ap();

	/* enter the scheduler */
	sched_throw(NULL);

	panic("scheduler returned us to %s", __func__);
	/* NOTREACHED */
}

static void
release_aps(void *dummy __unused)
{
	int ipi_irq;

	if (mp_ncpus == 1)
		return;

	/*
	 * IPI handler
	 */
	ipi_irq = platform_ipi_intrnum();
	cpu_establish_hardintr("ipi", mips_ipi_handler, NULL, NULL, ipi_irq,
			       INTR_TYPE_MISC | INTR_EXCL, NULL);

	atomic_store_rel_int(&aps_ready, 1);

	while (smp_started == 0)
		; /* nothing */
}

SYSINIT(start_aps, SI_SUB_SMP, SI_ORDER_FIRST, release_aps, NULL);