
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
#include <sys/proc.h>
#include <sys/bus.h>

#include <machine/intr_machdep.h>
#include <machine/apicvar.h>
#include <machine/segments.h>
#include <machine/md_var.h>

#include <machine/vmm.h>
#include "vmm_ipi.h"

extern inthand_t IDTVEC(rsvd), IDTVEC(justreturn);

/*
 * The default is to use the IPI_AST to interrupt a vcpu.
 */
int vmm_ipinum = IPI_AST;

CTASSERT(APIC_SPURIOUS_INT == 255);

void
vmm_ipi_init(void)
{
	int idx;
	uintptr_t func;
	struct gate_descriptor *ip;

	/*
	 * Search backwards from the highest IDT vector available for use
	 * as our IPI vector. We install the 'justreturn' handler at that
	 * vector and use it to interrupt the vcpus.
	 *
	 * We do this because the IPI_AST is heavyweight and saves all
	 * registers in the trapframe. This is overkill for our use case
	 * which is simply to EOI the interrupt and return.
	 */
	idx = APIC_SPURIOUS_INT;
	while (--idx >= APIC_IPI_INTS) {
		ip = &idt[idx];
		func = ((long)ip->gd_hioffset << 16 | ip->gd_looffset);
		if (func == (uintptr_t)&IDTVEC(rsvd)) {
			vmm_ipinum = idx;
			setidt(vmm_ipinum, IDTVEC(justreturn), SDT_SYSIGT,
			       SEL_KPL, 0);
			break;
		}
	}
	
	if (vmm_ipinum != IPI_AST && bootverbose) {
		printf("vmm_ipi_init: installing ipi handler to interrupt "
		       "vcpus at vector %d\n", vmm_ipinum);
	}
}

void
vmm_ipi_cleanup(void)
{
	if (vmm_ipinum != IPI_AST)
		setidt(vmm_ipinum, IDTVEC(rsvd), SDT_SYSIGT, SEL_KPL, 0);
}