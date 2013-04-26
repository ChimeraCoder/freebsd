
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
#include <sys/reboot.h>

#include <machine/machdep.h>

#include <dev/fdt/fdt_common.h>

#include <powerpc/mpc85xx/mpc85xx.h>

extern void dcache_enable(void);
extern void dcache_inval(void);
extern void icache_enable(void);
extern void icache_inval(void);
extern void l2cache_enable(void);
extern void l2cache_inval(void);

void
booke_init_tlb(vm_paddr_t fdt_immr_pa)
{

	/* Initialize TLB1 handling */
	tlb1_init(fdt_immr_pa);
}

void
booke_enable_l1_cache(void)
{
	uint32_t csr;

	/* Enable D-cache if applicable */
	csr = mfspr(SPR_L1CSR0);
	if ((csr & L1CSR0_DCE) == 0) {
		dcache_inval();
		dcache_enable();
	}

	csr = mfspr(SPR_L1CSR0);
	if ((boothowto & RB_VERBOSE) != 0 || (csr & L1CSR0_DCE) == 0)
		printf("L1 D-cache %sabled\n",
		    (csr & L1CSR0_DCE) ? "en" : "dis");

	/* Enable L1 I-cache if applicable. */
	csr = mfspr(SPR_L1CSR1);
	if ((csr & L1CSR1_ICE) == 0) {
		icache_inval();
		icache_enable();
	}

	csr = mfspr(SPR_L1CSR1);
	if ((boothowto & RB_VERBOSE) != 0 || (csr & L1CSR1_ICE) == 0)
		printf("L1 I-cache %sabled\n",
		    (csr & L1CSR1_ICE) ? "en" : "dis");
}

#if 0
void
booke_enable_l2_cache(void)
{
	uint32_t csr;

	/* Enable L2 cache on E500mc */
	if ((((mfpvr() >> 16) & 0xFFFF) == FSL_E500mc) ||
	    (((mfpvr() >> 16) & 0xFFFF) == FSL_E5500)) {
		csr = mfspr(SPR_L2CSR0);
		if ((csr & L2CSR0_L2E) == 0) {
			l2cache_inval();
			l2cache_enable();
		}

		csr = mfspr(SPR_L2CSR0);
		if ((boothowto & RB_VERBOSE) != 0 || (csr & L2CSR0_L2E) == 0)
			printf("L2 cache %sabled\n",
			    (csr & L2CSR0_L2E) ? "en" : "dis");
	}
}

void
booke_enable_l3_cache(void)
{
	uint32_t csr, size, ver;

	/* Enable L3 CoreNet Platform Cache (CPC) */
	ver = SVR_VER(mfspr(SPR_SVR));
	if (ver == SVR_P2041 || ver == SVR_P2041E || ver == SVR_P3041 ||
	    ver == SVR_P3041E || ver == SVR_P5020 || ver == SVR_P5020E) {
		csr = ccsr_read4(OCP85XX_CPC_CSR0);
		if ((csr & OCP85XX_CPC_CSR0_CE) == 0) {
			l3cache_inval();
			l3cache_enable();
		}

		csr = ccsr_read4(OCP85XX_CPC_CSR0);
		if ((boothowto & RB_VERBOSE) != 0 ||
		    (csr & OCP85XX_CPC_CSR0_CE) == 0) {
			size = OCP85XX_CPC_CFG0_SZ_K(ccsr_read4(OCP85XX_CPC_CFG0));
			printf("L3 Corenet Platform Cache: %d KB %sabled\n",
			    size, (csr & OCP85XX_CPC_CSR0_CE) == 0 ?
			    "dis" : "en");
		}
	}
}

void
booke_disable_l2_cache(void)
{
}

static void
l3cache_inval(void)
{

	/* Flash invalidate the CPC and clear all the locks */
	ccsr_write4(OCP85XX_CPC_CSR0, OCP85XX_CPC_CSR0_FI |
	    OCP85XX_CPC_CSR0_LFC);
	while (ccsr_read4(OCP85XX_CPC_CSR0) & (OCP85XX_CPC_CSR0_FI |
	    OCP85XX_CPC_CSR0_LFC))
		;
}

static void
l3cache_enable(void)
{

	ccsr_write4(OCP85XX_CPC_CSR0, OCP85XX_CPC_CSR0_CE |
	    OCP85XX_CPC_CSR0_PE);
	/* Read back to sync write */
	ccsr_read4(OCP85XX_CPC_CSR0);
}
#endif