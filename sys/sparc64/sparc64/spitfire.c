
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
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/smp.h>
#include <sys/sysctl.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/cache.h>
#include <machine/cpufunc.h>
#include <machine/lsu.h>
#include <machine/smp.h>
#include <machine/tlb.h>

#define	SPITFIRE_TLB_ENTRIES	64

PMAP_STATS_VAR(spitfire_dcache_npage_inval);
PMAP_STATS_VAR(spitfire_dcache_npage_inval_match);
PMAP_STATS_VAR(spitfire_icache_npage_inval);
PMAP_STATS_VAR(spitfire_icache_npage_inval_match);

/*
 * Enable the level 1 caches.
 */
void
spitfire_cache_enable(u_int cpu_impl __unused)
{
	u_long lsu;

	lsu = ldxa(0, ASI_LSU_CTL_REG);
	stxa_sync(0, ASI_LSU_CTL_REG, lsu | LSU_IC | LSU_DC);
}

/*
 * Flush all lines from the level 1 caches.
 */
void
spitfire_cache_flush(void)
{
	u_long addr;

	for (addr = 0; addr < PCPU_GET(cache.dc_size);
	    addr += PCPU_GET(cache.dc_linesize))
		stxa_sync(addr, ASI_DCACHE_TAG, 0);
	for (addr = 0; addr < PCPU_GET(cache.ic_size);
	    addr += PCPU_GET(cache.ic_linesize))
		stxa_sync(addr, ASI_ICACHE_TAG, 0);
}

/*
 * Flush a physical page from the data cache.
 */
void
spitfire_dcache_page_inval(vm_paddr_t pa)
{
	u_long target;
	void *cookie;
	u_long addr;
	u_long tag;

	KASSERT((pa & PAGE_MASK) == 0, ("%s: pa not page aligned", __func__));
	PMAP_STATS_INC(spitfire_dcache_npage_inval);
	target = pa >> (PAGE_SHIFT - DC_TAG_SHIFT);
	cookie = ipi_dcache_page_inval(tl_ipi_spitfire_dcache_page_inval, pa);
	for (addr = 0; addr < PCPU_GET(cache.dc_size);
	    addr += PCPU_GET(cache.dc_linesize)) {
		tag = ldxa(addr, ASI_DCACHE_TAG);
		if (((tag >> DC_VALID_SHIFT) & DC_VALID_MASK) == 0)
			continue;
		tag &= DC_TAG_MASK << DC_TAG_SHIFT;
		if (tag == target) {
			PMAP_STATS_INC(spitfire_dcache_npage_inval_match);
			stxa_sync(addr, ASI_DCACHE_TAG, tag);
		}
	}
	ipi_wait(cookie);
}

/*
 * Flush a physical page from the instruction cache.
 */
void
spitfire_icache_page_inval(vm_paddr_t pa)
{
	register u_long tag __asm("%g1");
	u_long target;
	void *cookie;
	u_long addr;

	KASSERT((pa & PAGE_MASK) == 0, ("%s: pa not page aligned", __func__));
	PMAP_STATS_INC(spitfire_icache_npage_inval);
	target = pa >> (PAGE_SHIFT - IC_TAG_SHIFT);
	cookie = ipi_icache_page_inval(tl_ipi_spitfire_icache_page_inval, pa);
	for (addr = 0; addr < PCPU_GET(cache.ic_size);
	    addr += PCPU_GET(cache.ic_linesize)) {
		__asm __volatile("ldda [%1] %2, %%g0" /*, %g1 */
		    : "=r" (tag) : "r" (addr), "n" (ASI_ICACHE_TAG));
		if (((tag >> IC_VALID_SHIFT) & IC_VALID_MASK) == 0)
			continue;
		tag &= IC_TAG_MASK << IC_TAG_SHIFT;
		if (tag == target) {
			PMAP_STATS_INC(spitfire_icache_npage_inval_match);
			stxa_sync(addr, ASI_ICACHE_TAG, tag);
		}
	}
	ipi_wait(cookie);
}

/*
 * Flush all non-locked mappings from the TLBs.
 */
void
spitfire_tlb_flush_nonlocked(void)
{
	u_int i;
	u_int slot;

	for (i = 0; i < SPITFIRE_TLB_ENTRIES; i++) {
		slot = TLB_DAR_SLOT(TLB_DAR_T32, i);
		if ((ldxa(slot, ASI_DTLB_DATA_ACCESS_REG) & TD_L) == 0)
			stxa_sync(slot, ASI_DTLB_DATA_ACCESS_REG, 0);
		if ((ldxa(slot, ASI_ITLB_DATA_ACCESS_REG) & TD_L) == 0)
			stxa_sync(slot, ASI_ITLB_DATA_ACCESS_REG, 0);
	}
}

/*
 * Flush all user mappings from the TLBs.
 */
void
spitfire_tlb_flush_user(void)
{
	u_long data;
	u_long tag;
	u_int i;
	u_int slot;

	for (i = 0; i < SPITFIRE_TLB_ENTRIES; i++) {
		slot = TLB_DAR_SLOT(TLB_DAR_T32, i);
		data = ldxa(slot, ASI_DTLB_DATA_ACCESS_REG);
		tag = ldxa(slot, ASI_DTLB_TAG_READ_REG);
		if ((data & TD_V) != 0 && (data & TD_L) == 0 &&
		    TLB_TAR_CTX(tag) != TLB_CTX_KERNEL)
			stxa_sync(slot, ASI_DTLB_DATA_ACCESS_REG, 0);
		data = ldxa(slot, ASI_ITLB_DATA_ACCESS_REG);
		tag = ldxa(slot, ASI_ITLB_TAG_READ_REG);
		if ((data & TD_V) != 0 && (data & TD_L) == 0 &&
		    TLB_TAR_CTX(tag) != TLB_CTX_KERNEL)
			stxa_sync(slot, ASI_ITLB_DATA_ACCESS_REG, 0);
	}
}