
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
#include <sys/bus.h>
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/smp.h>

#include <machine/pcb.h>
#include <machine/psl.h>
#include <machine/smp.h>
#include <machine/spr.h>

extern void dcache_enable(void);
extern void dcache_inval(void);
extern void icache_enable(void);
extern void icache_inval(void);

volatile void *ap_pcpu;

uintptr_t
cpudep_ap_bootstrap()
{
	uint32_t msr, sp, csr;

	/* Enable L1 caches */
	csr = mfspr(SPR_L1CSR0);
	if ((csr & L1CSR0_DCE) == 0) {
		dcache_inval();
		dcache_enable();
	}

	csr = mfspr(SPR_L1CSR1);
	if ((csr & L1CSR1_ICE) == 0) {
		icache_inval();
		icache_enable();
	}

	/* Set MSR */
	msr = PSL_ME;
	mtmsr(msr);

	/* Assign pcpu fields, return ptr to this AP's idle thread kstack */
	pcpup->pc_curthread = pcpup->pc_idlethread;
	pcpup->pc_curpcb = pcpup->pc_curthread->td_pcb;
	sp = pcpup->pc_curpcb->pcb_sp;

	/* XXX shouldn't the pcb_sp be checked/forced for alignment here?? */

	return (sp);
}

void
cpudep_ap_setup()
{
}