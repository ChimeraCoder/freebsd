
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

#include <stand.h>
#include <stdint.h>

#define _KERNEL
#include <machine/cpufunc.h>
#include <machine/psl.h>
#include <machine/pte.h>
#include <machine/slb.h>
#include <machine/param.h>

#include "bootstrap.h"
#include "lv1call.h"
#include "ps3.h"

register_t pteg_count, pteg_mask;
uint64_t as_id;
uint64_t virtual_avail;

int
ps3mmu_map(uint64_t va, uint64_t pa)
{
	struct lpte pt;
	int shift;
	uint64_t vsid, ptegidx;
	
	if (pa < 0x8000000) { /* Phys mem? */
		pt.pte_hi = LPTE_BIG;
		pt.pte_lo = LPTE_M;
		shift = 24;
		vsid = 0;
	} else {
		pt.pte_hi = 0;
		pt.pte_lo = LPTE_I | LPTE_G | LPTE_M | LPTE_NOEXEC;
		shift = ADDR_PIDX_SHFT;
		vsid = 1;
	}

	pt.pte_hi |= (vsid << LPTE_VSID_SHIFT) |
            (((uint64_t)(va & ADDR_PIDX) >> ADDR_API_SHFT64) & LPTE_API);
	pt.pte_lo |= pa;
	ptegidx = vsid ^ (((uint64_t)va & ADDR_PIDX) >> shift);

	pt.pte_hi |= LPTE_LOCKED | LPTE_VALID;
	ptegidx &= pteg_mask;

	return (lv1_insert_pte(ptegidx, &pt, LPTE_LOCKED));
}

void *
ps3mmu_mapdev(uint64_t pa, size_t length)
{
	uint64_t spa;
	void *mapstart;
	int err;
	
	mapstart = (void *)(uintptr_t)virtual_avail;

	for (spa = pa; spa < pa + length; spa += PAGE_SIZE) {
		err = ps3mmu_map(virtual_avail, spa);
		virtual_avail += PAGE_SIZE;
		if (err != 0)
			return (NULL);
	}

	return (mapstart);
}

int
ps3mmu_init(int maxmem)
{
	uint64_t ptsize;
	int i;

	i = lv1_setup_address_space(&as_id, &ptsize);
	pteg_count = ptsize / sizeof(struct lpteg);
	pteg_mask = pteg_count - 1;

	for (i = 0; i < maxmem; i += 16*1024*1024)
		ps3mmu_map(i,i);

	virtual_avail = 0x10000000;

	__asm __volatile ("slbia; slbmte %0, %1; slbmte %2,%3" ::
	    "r"((0 << SLBV_VSID_SHIFT) | SLBV_L), "r"(0 | SLBE_VALID),
	    "r"(1 << SLBV_VSID_SHIFT),
	    "r"((1 << SLBE_ESID_SHIFT) | SLBE_VALID | 1));

	mtmsr(PSL_IR | PSL_DR | PSL_RI | PSL_ME);

	return (0);
}