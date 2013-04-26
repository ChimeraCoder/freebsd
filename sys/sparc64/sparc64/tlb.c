
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

#include "opt_pmap.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ktr.h>
#include <sys/pcpu.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/smp.h>
#include <sys/sysctl.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/pmap.h>
#include <machine/smp.h>
#include <machine/tlb.h>
#include <machine/vmparam.h>

PMAP_STATS_VAR(tlb_ncontext_demap);
PMAP_STATS_VAR(tlb_npage_demap);
PMAP_STATS_VAR(tlb_nrange_demap);

tlb_flush_nonlocked_t *tlb_flush_nonlocked;
tlb_flush_user_t *tlb_flush_user;

/*
 * Some tlb operations must be atomic, so no interrupt or trap can be allowed
 * while they are in progress. Traps should not happen, but interrupts need to
 * be explicitely disabled. critical_enter() cannot be used here, since it only
 * disables soft interrupts.
 */

void
tlb_context_demap(struct pmap *pm)
{
	void *cookie;
	register_t s;

	/*
	 * It is important that we are not interrupted or preempted while
	 * doing the IPIs. The interrupted CPU may hold locks, and since
	 * it will wait for the CPU that sent the IPI, this can lead
	 * to a deadlock when an interrupt comes in on that CPU and it's
	 * handler tries to grab one of that locks. This will only happen for
	 * spin locks, but these IPI types are delivered even if normal
	 * interrupts are disabled, so the lock critical section will not
	 * protect the target processor from entering the IPI handler with
	 * the lock held.
	 */
	PMAP_STATS_INC(tlb_ncontext_demap);
	cookie = ipi_tlb_context_demap(pm);
	s = intr_disable();
	if (CPU_ISSET(PCPU_GET(cpuid), &pm->pm_active)) {
		KASSERT(pm->pm_context[curcpu] != -1,
		    ("tlb_context_demap: inactive pmap?"));
		stxa(TLB_DEMAP_PRIMARY | TLB_DEMAP_CONTEXT, ASI_DMMU_DEMAP, 0);
		stxa(TLB_DEMAP_PRIMARY | TLB_DEMAP_CONTEXT, ASI_IMMU_DEMAP, 0);
		flush(KERNBASE);
	}
	intr_restore(s);
	ipi_wait(cookie);
}

void
tlb_page_demap(struct pmap *pm, vm_offset_t va)
{
	u_long flags;
	void *cookie;
	register_t s;

	PMAP_STATS_INC(tlb_npage_demap);
	cookie = ipi_tlb_page_demap(pm, va);
	s = intr_disable();
	if (CPU_ISSET(PCPU_GET(cpuid), &pm->pm_active)) {
		KASSERT(pm->pm_context[curcpu] != -1,
		    ("tlb_page_demap: inactive pmap?"));
		if (pm == kernel_pmap)
			flags = TLB_DEMAP_NUCLEUS | TLB_DEMAP_PAGE;
		else
			flags = TLB_DEMAP_PRIMARY | TLB_DEMAP_PAGE;

		stxa(TLB_DEMAP_VA(va) | flags, ASI_DMMU_DEMAP, 0);
		stxa(TLB_DEMAP_VA(va) | flags, ASI_IMMU_DEMAP, 0);
		flush(KERNBASE);
	}
	intr_restore(s);
	ipi_wait(cookie);
}

void
tlb_range_demap(struct pmap *pm, vm_offset_t start, vm_offset_t end)
{
	vm_offset_t va;
	void *cookie;
	u_long flags;
	register_t s;

	PMAP_STATS_INC(tlb_nrange_demap);
	cookie = ipi_tlb_range_demap(pm, start, end);
	s = intr_disable();
	if (CPU_ISSET(PCPU_GET(cpuid), &pm->pm_active)) {
		KASSERT(pm->pm_context[curcpu] != -1,
		    ("tlb_range_demap: inactive pmap?"));
		if (pm == kernel_pmap)
			flags = TLB_DEMAP_NUCLEUS | TLB_DEMAP_PAGE;
		else
			flags = TLB_DEMAP_PRIMARY | TLB_DEMAP_PAGE;

		for (va = start; va < end; va += PAGE_SIZE) {
			stxa(TLB_DEMAP_VA(va) | flags, ASI_DMMU_DEMAP, 0);
			stxa(TLB_DEMAP_VA(va) | flags, ASI_IMMU_DEMAP, 0);
			flush(KERNBASE);
		}
	}
	intr_restore(s);
	ipi_wait(cookie);
}