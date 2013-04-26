
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>

#include <dev/pci/pcivar.h>

#include <dev/ppbus/ppbconf.h>
#include <dev/ppbus/ppb_msq.h>
#include <dev/ppc/ppcvar.h>
#include <dev/ppc/ppcreg.h>

#include "ppbus_if.h"

static int ppc_pci_probe(device_t dev);

static device_method_t ppc_pci_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		ppc_pci_probe),
	DEVMETHOD(device_attach,	ppc_attach),
	DEVMETHOD(device_detach,	ppc_detach),

	/* bus interface */
	DEVMETHOD(bus_read_ivar,	ppc_read_ivar),
	DEVMETHOD(bus_write_ivar,	ppc_write_ivar),
	DEVMETHOD(bus_alloc_resource,	ppc_alloc_resource),
	DEVMETHOD(bus_release_resource,	ppc_release_resource),

	/* ppbus interface */
	DEVMETHOD(ppbus_io,		ppc_io),
	DEVMETHOD(ppbus_exec_microseq,	ppc_exec_microseq),
	DEVMETHOD(ppbus_reset_epp,	ppc_reset_epp),
	DEVMETHOD(ppbus_setmode,	ppc_setmode),
	DEVMETHOD(ppbus_ecp_sync,	ppc_ecp_sync),
	DEVMETHOD(ppbus_read,		ppc_read),
	DEVMETHOD(ppbus_write,		ppc_write),

	{ 0, 0 }
};

static driver_t ppc_pci_driver = {
	ppc_driver_name,
	ppc_pci_methods,
	sizeof(struct ppc_data),
};

struct pci_id {
	uint32_t	type;
	const char	*desc;
	int		rid;
};

static struct pci_id pci_ids[] = {
	{ 0x1020131f, "SIIG Cyber Parallel PCI (10x family)", 0x18 },
	{ 0x2020131f, "SIIG Cyber Parallel PCI (20x family)", 0x10 },
	{ 0x05111407, "Lava SP BIDIR Parallel PCI", 0x10 },
	{ 0x80001407, "Lava Computers 2SP-PCI parallel port", 0x10 },
	{ 0x84031415, "Oxford Semiconductor OX12PCI840 Parallel port", 0x10 },
	{ 0x95131415, "Oxford Semiconductor OX16PCI954 Parallel port", 0x10 },
	{ 0x98059710, "NetMos NM9805 1284 Printer port", 0x10 },
	{ 0x98659710, "MosChip MCS9865 1284 Printer port", 0x10 },
	{ 0x99019710, "MosChip MCS9901 PCIe to Peripheral Controller", 0x10 },
	{ 0xffff }
};

static int
ppc_pci_probe(device_t dev)
{
	struct pci_id *id;
	uint32_t type;

	type = pci_get_devid(dev);
	id = pci_ids;
	while (id->type != 0xffff && id->type != type)
		id++;
	if (id->type == 0xffff)
		return (ENXIO);
	device_set_desc(dev, id->desc);
	return (ppc_probe(dev, id->rid));
}

DRIVER_MODULE(ppc, pci, ppc_pci_driver, ppc_devclass, 0, 0);