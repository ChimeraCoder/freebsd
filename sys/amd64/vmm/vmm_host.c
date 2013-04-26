
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
#include <sys/pcpu.h>

#include <machine/cpufunc.h>
#include <machine/segments.h>
#include <machine/specialreg.h>

#include "vmm_host.h"

static uint64_t vmm_host_efer, vmm_host_pat, vmm_host_cr0, vmm_host_cr4;

void
vmm_host_state_init(void)
{

	vmm_host_efer = rdmsr(MSR_EFER);
	vmm_host_pat = rdmsr(MSR_PAT);

	/*
	 * We always want CR0.TS to be set when the processor does a VM exit.
	 *
	 * With emulation turned on unconditionally after a VM exit, we are
	 * able to trap inadvertent use of the FPU until the guest FPU state
	 * has been safely squirreled away.
	 */
	vmm_host_cr0 = rcr0() | CR0_TS;

	vmm_host_cr4 = rcr4();
}

uint64_t
vmm_get_host_pat(void)
{

	return (vmm_host_pat);
}

uint64_t
vmm_get_host_efer(void)
{

	return (vmm_host_efer);
}

uint64_t
vmm_get_host_cr0(void)
{

	return (vmm_host_cr0);
}

uint64_t
vmm_get_host_cr4(void)
{

	return (vmm_host_cr4);
}

uint64_t
vmm_get_host_datasel(void)
{

	return (GSEL(GDATA_SEL, SEL_KPL));

}

uint64_t
vmm_get_host_codesel(void)
{

	return (GSEL(GCODE_SEL, SEL_KPL));
}

uint64_t
vmm_get_host_tsssel(void)
{

	return (GSEL(GPROC0_SEL, SEL_KPL));
}

uint64_t
vmm_get_host_fsbase(void)
{

	return (0);
}

uint64_t
vmm_get_host_idtrbase(void)
{

	return (r_idt.rd_base);
}