
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

#include <machine/asi.h>
#include <machine/cache.h>
#include <machine/cpu.h>
#include <machine/cpufunc.h>
#include <machine/mcntl.h>
#include <machine/lsu.h>
#include <machine/tlb.h>
#include <machine/tte.h>
#include <machine/vmparam.h>

#define	ZEUS_FTLB_ENTRIES	32
#define	ZEUS_STLB_ENTRIES	2048

/*
 * CPU-specific initialization for Fujitsu Zeus CPUs
 */
void
zeus_init(u_int cpu_impl)
{
	u_long val;

	/* Ensure the TSB Extension Registers hold 0 as TSB_Base. */

	stxa(AA_DMMU_TSB_PEXT_REG, ASI_DMMU, 0);
	stxa(AA_IMMU_TSB_PEXT_REG, ASI_IMMU, 0);
	membar(Sync);

	stxa(AA_DMMU_TSB_SEXT_REG, ASI_DMMU, 0);
	/*
	 * NB: the secondary context was removed from the iMMU.
	 */
	membar(Sync);

	stxa(AA_DMMU_TSB_NEXT_REG, ASI_DMMU, 0);
	stxa(AA_IMMU_TSB_NEXT_REG, ASI_IMMU, 0);
	membar(Sync);

	val = ldxa(AA_MCNTL, ASI_MCNTL);
	/* Ensure MCNTL_JPS1_TSBP is 0. */
	val &= ~MCNTL_JPS1_TSBP;
	/*
	 * Ensure 4-Mbyte page entries are stored in the 1024-entry, 2-way set
	 * associative TLB.
	 */
	val = (val & ~MCNTL_RMD_MASK) | MCNTL_RMD_1024;
	stxa(AA_MCNTL, ASI_MCNTL, val);
}

/*
 * Enable level 1 caches.
 */
void
zeus_cache_enable(u_int cpu_impl)
{
	u_long lsu;

	lsu = ldxa(0, ASI_LSU_CTL_REG);
	stxa(0, ASI_LSU_CTL_REG, lsu | LSU_IC | LSU_DC);
	flush(KERNBASE);
}

/*
 * Flush all lines from the level 1 caches.
 */
void
zeus_cache_flush(void)
{

	stxa_sync(0, ASI_FLUSH_L1I, 0);
}

/*
 * Flush a physical page from the data cache.  Data cache consistency is
 * maintained by hardware.
 */
void
zeus_dcache_page_inval(vm_paddr_t spa __unused)
{

}

/*
 * Flush a physical page from the intsruction cache.  Instruction cache
 * consistency is maintained by hardware.
 */
void
zeus_icache_page_inval(vm_paddr_t pa __unused)
{

}

/*
 * Flush all non-locked mappings from the TLBs.
 */
void
zeus_tlb_flush_nonlocked(void)
{

	stxa(TLB_DEMAP_ALL, ASI_DMMU_DEMAP, 0);
	stxa(TLB_DEMAP_ALL, ASI_IMMU_DEMAP, 0);
	flush(KERNBASE);
}

/*
 * Flush all user mappings from the TLBs.
 */
void
zeus_tlb_flush_user(void)
{
	u_long data, tag;
	u_int i, slot;

	for (i = 0; i < ZEUS_FTLB_ENTRIES; i++) {
		slot = TLB_DAR_SLOT(TLB_DAR_FTLB, i);
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
	for (i = 0; i < ZEUS_STLB_ENTRIES; i++) {
		slot = TLB_DAR_SLOT(TLB_DAR_STLB, i);
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