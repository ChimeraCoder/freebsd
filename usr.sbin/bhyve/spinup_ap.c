
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
#include <sys/types.h>

#include <machine/vmm.h>
#include <vmmapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bhyverun.h"
#include "spinup_ap.h"

static void
spinup_ap_realmode(struct vmctx *ctx, int newcpu, uint64_t *rip)
{
	int vector, error;
	uint16_t cs;
	uint64_t desc_base;
	uint32_t desc_limit, desc_access;

	vector = *rip >> PAGE_SHIFT;
	*rip = 0;

	/*
	 * Update the %cs and %rip of the guest so that it starts
	 * executing real mode code at at 'vector << 12'.
	 */
	error = vm_set_register(ctx, newcpu, VM_REG_GUEST_RIP, *rip);
	assert(error == 0);

	error = vm_get_desc(ctx, newcpu, VM_REG_GUEST_CS, &desc_base,
			    &desc_limit, &desc_access);
	assert(error == 0);

	desc_base = vector << PAGE_SHIFT;
	error = vm_set_desc(ctx, newcpu, VM_REG_GUEST_CS,
			    desc_base, desc_limit, desc_access);
	assert(error == 0);

	cs = (vector << PAGE_SHIFT) >> 4;
	error = vm_set_register(ctx, newcpu, VM_REG_GUEST_CS, cs);
	assert(error == 0);
}

int
spinup_ap(struct vmctx *ctx, int vcpu, int newcpu, uint64_t rip)
{
	int error;

	assert(newcpu != 0);
	assert(newcpu < guest_ncpus);

	error = vcpu_reset(ctx, newcpu);
	assert(error == 0);

	/* Set up capabilities */
	if (fbsdrun_vmexit_on_hlt()) {
		error = vm_set_capability(ctx, newcpu, VM_CAP_HALT_EXIT, 1);
		assert(error == 0);
	}

	if (fbsdrun_vmexit_on_pause()) {
		error = vm_set_capability(ctx, newcpu, VM_CAP_PAUSE_EXIT, 1);
		assert(error == 0);
	}

	if (fbsdrun_disable_x2apic())
		error = vm_set_x2apic_state(ctx, newcpu, X2APIC_DISABLED);
	else
		error = vm_set_x2apic_state(ctx, newcpu, X2APIC_ENABLED);
	assert(error == 0);

	/*
	 * Enable the 'unrestricted guest' mode for 'newcpu'.
	 *
	 * Set up the processor state in power-on 16-bit mode, with the CS:IP
	 * init'd to the specified low-mem 4K page.
	 */
	error = vm_set_capability(ctx, newcpu, VM_CAP_UNRESTRICTED_GUEST, 1);
	assert(error == 0);

	spinup_ap_realmode(ctx, newcpu, &rip);

	fbsdrun_addcpu(ctx, newcpu, rip);

	return (newcpu);
}