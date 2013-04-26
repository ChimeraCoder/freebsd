
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
#include <sys/systm.h>
#include <sys/bus.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

#include <machine/md_var.h>

#include "vmm_util.h"
#include "vmm_mem.h"
#include "iommu.h"

static boolean_t iommu_avail;
static struct iommu_ops *ops;
static void *host_domain;

static __inline int
IOMMU_INIT(void)
{
	if (ops != NULL)
		return ((*ops->init)());
	else
		return (ENXIO);
}

static __inline void
IOMMU_CLEANUP(void)
{
	if (ops != NULL && iommu_avail)
		(*ops->cleanup)();
}

static __inline void *
IOMMU_CREATE_DOMAIN(vm_paddr_t maxaddr)
{

	if (ops != NULL && iommu_avail)
		return ((*ops->create_domain)(maxaddr));
	else
		return (NULL);
}

static __inline void
IOMMU_DESTROY_DOMAIN(void *dom)
{

	if (ops != NULL && iommu_avail)
		(*ops->destroy_domain)(dom);
}

static __inline uint64_t
IOMMU_CREATE_MAPPING(void *domain, vm_paddr_t gpa, vm_paddr_t hpa, uint64_t len)
{

	if (ops != NULL && iommu_avail)
		return ((*ops->create_mapping)(domain, gpa, hpa, len));
	else
		return (len);		/* XXX */
}

static __inline uint64_t
IOMMU_REMOVE_MAPPING(void *domain, vm_paddr_t gpa, uint64_t len)
{

	if (ops != NULL && iommu_avail)
		return ((*ops->remove_mapping)(domain, gpa, len));
	else
		return (len);		/* XXX */
}

static __inline void
IOMMU_ADD_DEVICE(void *domain, int bus, int slot, int func)
{

	if (ops != NULL && iommu_avail)
		(*ops->add_device)(domain, bus, slot, func);
}

static __inline void
IOMMU_REMOVE_DEVICE(void *domain, int bus, int slot, int func)
{

	if (ops != NULL && iommu_avail)
		(*ops->remove_device)(domain, bus, slot, func);
}

static __inline void
IOMMU_INVALIDATE_TLB(void *domain)
{

	if (ops != NULL && iommu_avail)
		(*ops->invalidate_tlb)(domain);
}

static __inline void
IOMMU_ENABLE(void)
{

	if (ops != NULL && iommu_avail)
		(*ops->enable)();
}

static __inline void
IOMMU_DISABLE(void)
{

	if (ops != NULL && iommu_avail)
		(*ops->disable)();
}

void
iommu_init(void)
{
	int error, bus, slot, func;
	vm_paddr_t maxaddr;
	const char *name;
	device_t dev;

	if (vmm_is_intel())
		ops = &iommu_ops_intel;
	else if (vmm_is_amd())
		ops = &iommu_ops_amd;
	else
		ops = NULL;

	error = IOMMU_INIT();
	if (error)
		return;

	iommu_avail = TRUE;

	/*
	 * Create a domain for the devices owned by the host
	 */
	maxaddr = vmm_mem_maxaddr();
	host_domain = IOMMU_CREATE_DOMAIN(maxaddr);
	if (host_domain == NULL)
		panic("iommu_init: unable to create a host domain");

	/*
	 * Create 1:1 mappings from '0' to 'maxaddr' for devices assigned to
	 * the host
	 */
	iommu_create_mapping(host_domain, 0, 0, maxaddr);

	for (bus = 0; bus <= PCI_BUSMAX; bus++) {
		for (slot = 0; slot <= PCI_SLOTMAX; slot++) {
			for (func = 0; func <= PCI_FUNCMAX; func++) {
				dev = pci_find_dbsf(0, bus, slot, func);
				if (dev == NULL)
					continue;

				/* skip passthrough devices */
				name = device_get_name(dev);
				if (name != NULL && strcmp(name, "ppt") == 0)
					continue;

				/* everything else belongs to the host domain */
				iommu_add_device(host_domain, bus, slot, func);
			}
		}
	}
	IOMMU_ENABLE();

}

void
iommu_cleanup(void)
{
	IOMMU_DISABLE();
	IOMMU_DESTROY_DOMAIN(host_domain);
	IOMMU_CLEANUP();
}

void *
iommu_create_domain(vm_paddr_t maxaddr)
{

	return (IOMMU_CREATE_DOMAIN(maxaddr));
}

void
iommu_destroy_domain(void *dom)
{

	IOMMU_DESTROY_DOMAIN(dom);
}

void
iommu_create_mapping(void *dom, vm_paddr_t gpa, vm_paddr_t hpa, size_t len)
{
	uint64_t mapped, remaining;

	remaining = len;

	while (remaining > 0) {
		mapped = IOMMU_CREATE_MAPPING(dom, gpa, hpa, remaining);
		gpa += mapped;
		hpa += mapped;
		remaining -= mapped;
	}
}

void
iommu_remove_mapping(void *dom, vm_paddr_t gpa, size_t len)
{
	uint64_t unmapped, remaining;

	remaining = len;

	while (remaining > 0) {
		unmapped = IOMMU_REMOVE_MAPPING(dom, gpa, remaining);
		gpa += unmapped;
		remaining -= unmapped;
	}
}

void *
iommu_host_domain(void)
{

	return (host_domain);
}

void
iommu_add_device(void *dom, int bus, int slot, int func)
{

	IOMMU_ADD_DEVICE(dom, bus, slot, func);
}

void
iommu_remove_device(void *dom, int bus, int slot, int func)
{

	IOMMU_REMOVE_DEVICE(dom, bus, slot, func);
}

void
iommu_invalidate_tlb(void *domain)
{

	IOMMU_INVALIDATE_TLB(domain);
}