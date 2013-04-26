
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

#include <machine/specialreg.h>
#include <machine/segments.h>
#include <machine/vmm.h>

#include "vmmapi.h"

#define	DESC_UNUSABLE		0x00010000

#define	GUEST_NULL_SEL		0
#define	GUEST_CODE_SEL		1
#define	GUEST_DATA_SEL		2
#define	GUEST_GDTR_LIMIT	(3 * 8 - 1)

void     
vm_setup_freebsd_gdt(uint64_t *gdtr)
{       
	gdtr[GUEST_NULL_SEL] = 0;
	gdtr[GUEST_CODE_SEL] = 0x0020980000000000;
	gdtr[GUEST_DATA_SEL] = 0x0000900000000000;
}

/*
 * Setup the 'vcpu' register set such that it will begin execution at
 * 'rip' in long mode.
 */
int
vm_setup_freebsd_registers(struct vmctx *vmctx, int vcpu,
			   uint64_t rip, uint64_t cr3, uint64_t gdtbase,
			   uint64_t rsp)
{
	int error;
	uint64_t cr0, cr4, efer, rflags, desc_base;
	uint32_t desc_access, desc_limit;
	uint16_t gsel;

	cr0 = CR0_PE | CR0_PG | CR0_NE;
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_CR0, cr0)) != 0)
		goto done;

	cr4 = CR4_PAE;
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_CR4, cr4)) != 0)
		goto done;

	efer = EFER_LME | EFER_LMA;
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_EFER, efer)))
		goto done;

	rflags = 0x2;
	error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_RFLAGS, rflags);
	if (error)
		goto done;

	desc_base = 0;
	desc_limit = 0;
	desc_access = 0x0000209B;
	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_CS,
			    desc_base, desc_limit, desc_access);
	if (error)
		goto done;

	desc_access = 0x00000093;
	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_DS,
			    desc_base, desc_limit, desc_access);
	if (error)
		goto done;

	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_ES,
			    desc_base, desc_limit, desc_access);
	if (error)
		goto done;

	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_FS,
			    desc_base, desc_limit, desc_access);
	if (error)
		goto done;

	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_GS,
			    desc_base, desc_limit, desc_access);
	if (error)
		goto done;

	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_SS,
			    desc_base, desc_limit, desc_access);
	if (error)
		goto done;

	/*
	 * XXX TR is pointing to null selector even though we set the
	 * TSS segment to be usable with a base address and limit of 0.
	 */
	desc_access = 0x0000008b;
	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_TR, 0, 0, desc_access);
	if (error)
		goto done;

	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_LDTR, 0, 0,
			    DESC_UNUSABLE);
	if (error)
		goto done;

	gsel = GSEL(GUEST_CODE_SEL, SEL_KPL);
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_CS, gsel)) != 0)
		goto done;
	
	gsel = GSEL(GUEST_DATA_SEL, SEL_KPL);
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_DS, gsel)) != 0)
		goto done;
	
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_ES, gsel)) != 0)
		goto done;

	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_FS, gsel)) != 0)
		goto done;
	
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_GS, gsel)) != 0)
		goto done;
	
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_SS, gsel)) != 0)
		goto done;

	/* XXX TR is pointing to the null selector */
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_TR, 0)) != 0)
		goto done;

	/* LDTR is pointing to the null selector */
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_LDTR, 0)) != 0)
		goto done;

	/* entry point */
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_RIP, rip)) != 0)
		goto done;

	/* page table base */
	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_CR3, cr3)) != 0)
		goto done;

	desc_base = gdtbase;
	desc_limit = GUEST_GDTR_LIMIT;
	error = vm_set_desc(vmctx, vcpu, VM_REG_GUEST_GDTR,
			    desc_base, desc_limit, 0);
	if (error != 0)
		goto done;

	if ((error = vm_set_register(vmctx, vcpu, VM_REG_GUEST_RSP, rsp)) != 0)
		goto done;

	error = 0;
done:
	return (error);
}