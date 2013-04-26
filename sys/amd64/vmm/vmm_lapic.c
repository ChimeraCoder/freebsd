
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
#include <sys/smp.h>

#include <x86/specialreg.h>
#include <x86/apicreg.h>

#include <machine/vmm.h>
#include "vmm_ipi.h"
#include "vmm_lapic.h"
#include "vlapic.h"

int
lapic_pending_intr(struct vm *vm, int cpu)
{
	struct vlapic *vlapic;

	vlapic = vm_lapic(vm, cpu);

	return (vlapic_pending_intr(vlapic));
}

void
lapic_intr_accepted(struct vm *vm, int cpu, int vector)
{
	struct vlapic *vlapic;

	vlapic = vm_lapic(vm, cpu);

	vlapic_intr_accepted(vlapic, vector);
}

int
lapic_set_intr(struct vm *vm, int cpu, int vector)
{
	struct vlapic *vlapic;

	if (cpu < 0 || cpu >= VM_MAXCPU)
		return (EINVAL);

	if (vector < 32 || vector > 255)
		return (EINVAL);

	vlapic = vm_lapic(vm, cpu);
	vlapic_set_intr_ready(vlapic, vector);

	vm_interrupt_hostcpu(vm, cpu);

	return (0);
}

int
lapic_timer_tick(struct vm *vm, int cpu)
{
	struct vlapic *vlapic;

	vlapic = vm_lapic(vm, cpu);

	return (vlapic_timer_tick(vlapic));
}

static boolean_t
x2apic_msr(u_int msr)
{
	if (msr >= 0x800 && msr <= 0xBFF)
		return (TRUE);
	else
		return (FALSE);
}

static u_int
x2apic_msr_to_regoff(u_int msr)
{

	return ((msr - 0x800) << 4);
}

boolean_t
lapic_msr(u_int msr)
{

	if (x2apic_msr(msr) || (msr == MSR_APICBASE))
		return (TRUE);
	else
		return (FALSE);
}

int
lapic_rdmsr(struct vm *vm, int cpu, u_int msr, uint64_t *rval)
{
	int error;
	u_int offset;
	struct vlapic *vlapic;

	vlapic = vm_lapic(vm, cpu);

	if (msr == MSR_APICBASE) {
		*rval = vlapic_get_apicbase(vlapic);
		error = 0;
	} else {
		offset = x2apic_msr_to_regoff(msr);
		error = vlapic_op_mem_read(vlapic, offset, DWORD, rval);
	}

	return (error);
}

int
lapic_wrmsr(struct vm *vm, int cpu, u_int msr, uint64_t val)
{
	int error;
	u_int offset;
	struct vlapic *vlapic;

	vlapic = vm_lapic(vm, cpu);

	if (msr == MSR_APICBASE) {
		vlapic_set_apicbase(vlapic, val);
		error = 0;
	} else {
		offset = x2apic_msr_to_regoff(msr);
		error = vlapic_op_mem_write(vlapic, offset, DWORD, val);
	}

	return (error);
}

int
lapic_mmio_write(void *vm, int cpu, uint64_t gpa, uint64_t wval, int size,
		 void *arg)
{
	int error;
	uint64_t off;
	struct vlapic *vlapic;

	off = gpa - DEFAULT_APIC_BASE;

	/*
	 * Memory mapped local apic accesses must be 4 bytes wide and
	 * aligned on a 16-byte boundary.
	 */
	if (size != 4 || off & 0xf)
		return (EINVAL);

	vlapic = vm_lapic(vm, cpu);
	error = vlapic_op_mem_write(vlapic, off, DWORD, wval);
	return (error);
}

int
lapic_mmio_read(void *vm, int cpu, uint64_t gpa, uint64_t *rval, int size,
		void *arg)
{
	int error;
	uint64_t off;
	struct vlapic *vlapic;

	off = gpa - DEFAULT_APIC_BASE;

	/*
	 * Memory mapped local apic accesses must be 4 bytes wide and
	 * aligned on a 16-byte boundary.
	 */
	if (size != 4 || off & 0xf)
		return (EINVAL);

	vlapic = vm_lapic(vm, cpu);
	error = vlapic_op_mem_read(vlapic, off, DWORD, rval);
	return (error);
}