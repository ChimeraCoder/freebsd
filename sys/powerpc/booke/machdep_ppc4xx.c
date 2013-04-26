
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

#include <sys/types.h>
#include <sys/systm.h>

#include <machine/machdep.h>

#include <powerpc/booke/dcr.h>
#include <powerpc/apm86xxx/apm86xxx.h>

#include <dev/fdt/fdt_common.h>

#define OCP_ADDR_WORDLO(addr)	((uint32_t)((uint64_t)(addr) & 0xFFFFFFFF))
#define OCP_ADDR_WORDHI(addr)	((uint32_t)((uint64_t)(addr) >> 32))

extern void tlb_write(u_int, uint32_t, uint32_t, uint32_t, tlbtid_t, uint32_t,
    uint32_t);
extern void tlb_read(u_int, uint32_t *, uint32_t *, uint32_t *, uint32_t *,
    uint32_t *, uint32_t *);

unsigned int tlb_static_entries;
unsigned int tlb_current_entry = TLB_SIZE;
unsigned int tlb_misses = 0;
unsigned int tlb_invals = 0;

void tlb_map(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
void tlb_map_mem(uint32_t, uint32_t, uint32_t);
void tlb_dump(void);

void
booke_init_tlb(vm_paddr_t fdt_immr_pa)
{

	/* Map register space */
	tlb_map(APM86XXX_DEEP_SLEEP_VA,
	    OCP_ADDR_WORDLO(APM86XXX_DEEP_SLEEP_PA),
	    OCP_ADDR_WORDHI(APM86XXX_DEEP_SLEEP_PA), TLB_VALID | TLB_SIZE_16M,
	    TLB_SW | TLB_SR | TLB_I | TLB_G);

	tlb_map(APM86XXX_CSR_VA, OCP_ADDR_WORDLO(APM86XXX_CSR_PA),
	    OCP_ADDR_WORDHI(APM86XXX_CSR_PA), TLB_VALID | TLB_SIZE_16M,
	    TLB_SW | TLB_SR | TLB_I | TLB_G);

	tlb_map(APM86XXX_PRIMARY_FABRIC_VA,
	    OCP_ADDR_WORDLO(APM86XXX_PRIMARY_FABRIC_PA),
	    OCP_ADDR_WORDHI(APM86XXX_PRIMARY_FABRIC_PA),
	    TLB_VALID | TLB_SIZE_16M,
	    TLB_SW | TLB_SR | TLB_I | TLB_G);

	tlb_map(APM86XXX_AHB_VA, OCP_ADDR_WORDLO(APM86XXX_AHB_PA),
	    OCP_ADDR_WORDHI(APM86XXX_AHB_PA),
	    TLB_VALID | TLB_SIZE_16M,
	    TLB_SW | TLB_SR | TLB_I | TLB_G);

	/* Map MailBox space */
	tlb_map(APM86XXX_MBOX_VA, OCP_ADDR_WORDLO(APM86XXX_MBOX_PA),
	    OCP_ADDR_WORDHI(APM86XXX_MBOX_PA),
	    TLB_VALID | TLB_SIZE_4K,
	    TLB_UX | TLB_UW | TLB_UR |
	    TLB_SX | TLB_SW | TLB_SR |
	    TLB_I | TLB_G);

	tlb_map(APM86XXX_MBOX_VA + 0x1000,
	    OCP_ADDR_WORDLO(APM86XXX_MBOX_PA) + 0x1000,
	    OCP_ADDR_WORDHI(APM86XXX_MBOX_PA),
	    TLB_VALID | TLB_SIZE_4K,
	    TLB_UX | TLB_UW | TLB_UR |
	    TLB_SX | TLB_SW | TLB_SR |
	    TLB_I | TLB_G);

	tlb_map(APM86XXX_MBOX_VA + 0x2000,
	    OCP_ADDR_WORDLO(APM86XXX_MBOX_PA)+ 0x2000,
	    OCP_ADDR_WORDHI(APM86XXX_MBOX_PA),
	    TLB_VALID | TLB_SIZE_4K,
	    TLB_UX | TLB_UW | TLB_UR |
	    TLB_SX | TLB_SW | TLB_SR |
	    TLB_I | TLB_G);
}

void
booke_enable_l1_cache(void)
{
}

void
booke_enable_l2_cache(void)
{
}

void
booke_enable_l3_cache(void)
{
}

void
booke_disable_l2_cache(void)
{
	uint32_t ccr1,l2cr0;

	/* Disable L2 cache op broadcast */
	ccr1 = mfspr(SPR_CCR1);
	ccr1 &= ~CCR1_L2COBE;
	mtspr(SPR_CCR1, ccr1);

	/* Set L2 array size to 0 i.e. disable L2 cache */
	mtdcr(DCR_L2DCDCRAI, DCR_L2CR0);
	l2cr0 = mfdcr(DCR_L2DCDCRDI);
	l2cr0 &= ~L2CR0_AS;
	mtdcr(DCR_L2DCDCRDI, l2cr0);
}

void tlb_map(uint32_t epn, uint32_t rpn, uint32_t erpn, uint32_t flags,
    uint32_t perms)
{

	tlb_write(++tlb_static_entries, epn, rpn, erpn, 0, flags, perms);
}

static void tlb_dump_entry(u_int entry)
{
	uint32_t epn, rpn, erpn, tid, flags, perms;
	const char *size;

	tlb_read(entry, &epn, &rpn, &erpn, &tid, &flags, &perms);

	switch (flags & TLB_SIZE_MASK) {
	case TLB_SIZE_1K:
		size = "  1k";
		break;
	case TLB_SIZE_4K:
		size = "  4k";
		break;
	case TLB_SIZE_16K:
		size = " 16k";
		break;
	case TLB_SIZE_256K:
		size = "256k";
		break;
	case TLB_SIZE_1M:
		size = "  1M";
		break;
	case TLB_SIZE_16M:
		size = " 16M";
		break;
	case TLB_SIZE_256M:
		size = "256M";
		break;
	case TLB_SIZE_1G:
		size = "  1G";
		break;
	default:
		size = "????";
		break;
	}


	printf("TLB[%02u]: 0x%08X => "
	    "0x%01X_%08X %s %c %c %s %s %s %s %s "
	    "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c (%u)\n",
	    entry, epn, erpn, rpn, size,
	    (flags & TLB_TS)	? '1'		: '0',
	    (flags & TLB_VALID)	?  'V'		: '.',
	    (perms & TLB_WL1)	? "WL1"		: "___",
	    (perms & TLB_IL1I)	? "IL1I"	: "____",
	    (perms & TLB_IL1D)	? "IL1D"	: "____",
	    (perms & TLB_IL2I)	? "IL2I"	: "____",
	    (perms & TLB_IL2D)	? "IL2D"	: "____",
	    (perms & TLB_U0)	? '1'		: '.',
	    (perms & TLB_U1)	? '2'		: '.',
	    (perms & TLB_U2)	? '3'		: '.',
	    (perms & TLB_U3)	? '4'		: '.',
	    (perms & TLB_W)		? 'W'		: '.',
	    (perms & TLB_I)		? 'I'		: '.',
	    (perms & TLB_M)		? 'M'		: '.',
	    (perms & TLB_G)		? 'G'		: '.',
	    (perms & TLB_E)		? 'E'		: '.',
	    (perms & TLB_UX)	? 'x'		: '.',
	    (perms & TLB_UW)	? 'w'		: '.',
	    (perms & TLB_UR)	? 'r'		: '.',
	    (perms & TLB_SX)	? 'X'		: '.',
	    (perms & TLB_SW)	? 'W'		: '.',
	    (perms & TLB_SR)	? 'R'		: '.',
	    tid);
}

void tlb_dump(void)
{
	int i;

	for (i = 0; i < TLB_SIZE; i++)
		tlb_dump_entry(i);
}