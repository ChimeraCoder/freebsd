
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
#include <sys/errno.h>
#include <sys/smp.h>

#include <machine/vmm.h>
#include "io/iommu.h"

static int
amdv_init(void)
{

	printf("amdv_init: not implemented\n");
	return (ENXIO);
}

static int
amdv_cleanup(void)
{

	printf("amdv_cleanup: not implemented\n");
	return (ENXIO);
}

static void *
amdv_vminit(struct vm *vm)
{

	printf("amdv_vminit: not implemented\n");
	return (NULL);
}

static int
amdv_vmrun(void *arg, int vcpu, register_t rip)
{

	printf("amdv_vmrun: not implemented\n");
	return (ENXIO);
}

static void
amdv_vmcleanup(void *arg)
{

	printf("amdv_vmcleanup: not implemented\n");
	return;
}

static int
amdv_vmmmap_set(void *arg, vm_paddr_t gpa, vm_paddr_t hpa, size_t length,
	    vm_memattr_t attr, int prot, boolean_t spok)
{

	printf("amdv_vmmmap_set: not implemented\n");
	return (EINVAL);
}

static vm_paddr_t
amdv_vmmmap_get(void *arg, vm_paddr_t gpa)
{

	printf("amdv_vmmmap_get: not implemented\n");
	return (EINVAL);
}

static int
amdv_getreg(void *arg, int vcpu, int regnum, uint64_t *retval)
{
	
	printf("amdv_getreg: not implemented\n");
	return (EINVAL);
}

static int
amdv_setreg(void *arg, int vcpu, int regnum, uint64_t val)
{
	
	printf("amdv_setreg: not implemented\n");
	return (EINVAL);
}

static int
amdv_getdesc(void *vmi, int vcpu, int num, struct seg_desc *desc)
{

	printf("amdv_get_desc: not implemented\n");
	return (EINVAL);
}

static int
amdv_setdesc(void *vmi, int vcpu, int num, struct seg_desc *desc)
{

	printf("amdv_get_desc: not implemented\n");
	return (EINVAL);
}

static int
amdv_inject_event(void *vmi, int vcpu, int type, int vector,
		  uint32_t error_code, int error_code_valid)
{

	printf("amdv_inject_event: not implemented\n");
	return (EINVAL);
}

static int
amdv_getcap(void *arg, int vcpu, int type, int *retval)
{

	printf("amdv_getcap: not implemented\n");
	return (EINVAL);
}

static int
amdv_setcap(void *arg, int vcpu, int type, int val)
{

	printf("amdv_setcap: not implemented\n");
	return (EINVAL);
}

struct vmm_ops vmm_ops_amd = {
	amdv_init,
	amdv_cleanup,
	amdv_vminit,
	amdv_vmrun,
	amdv_vmcleanup,
	amdv_vmmmap_set,
	amdv_vmmmap_get,
	amdv_getreg,
	amdv_setreg,
	amdv_getdesc,
	amdv_setdesc,
	amdv_inject_event,
	amdv_getcap,
	amdv_setcap
};

static int
amd_iommu_init(void)
{

	printf("amd_iommu_init: not implemented\n");
	return (ENXIO);
}

static void
amd_iommu_cleanup(void)
{

	printf("amd_iommu_cleanup: not implemented\n");
}

static void
amd_iommu_enable(void)
{

	printf("amd_iommu_enable: not implemented\n");
}

static void
amd_iommu_disable(void)
{

	printf("amd_iommu_disable: not implemented\n");
}

static void *
amd_iommu_create_domain(vm_paddr_t maxaddr)
{

	printf("amd_iommu_create_domain: not implemented\n");
	return (NULL);
}

static void
amd_iommu_destroy_domain(void *domain)
{

	printf("amd_iommu_destroy_domain: not implemented\n");
}

static uint64_t
amd_iommu_create_mapping(void *domain, vm_paddr_t gpa, vm_paddr_t hpa,
			 uint64_t len)
{

	printf("amd_iommu_create_mapping: not implemented\n");
	return (0);
}

static uint64_t
amd_iommu_remove_mapping(void *domain, vm_paddr_t gpa, uint64_t len)
{

	printf("amd_iommu_remove_mapping: not implemented\n");
	return (0);
}

static void
amd_iommu_add_device(void *domain, int bus, int slot, int func)
{

	printf("amd_iommu_add_device: not implemented\n");
}

static void
amd_iommu_remove_device(void *domain, int bus, int slot, int func)
{

	printf("amd_iommu_remove_device: not implemented\n");
}

static void
amd_iommu_invalidate_tlb(void *domain)
{

	printf("amd_iommu_invalidate_tlb: not implemented\n");
}

struct iommu_ops iommu_ops_amd = {
	amd_iommu_init,
	amd_iommu_cleanup,
	amd_iommu_enable,
	amd_iommu_disable,
	amd_iommu_create_domain,
	amd_iommu_destroy_domain,
	amd_iommu_create_mapping,
	amd_iommu_remove_mapping,
	amd_iommu_add_device,
	amd_iommu_remove_device,
	amd_iommu_invalidate_tlb,
};