
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
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>

#include <cam/scsi/scsi_all.h>

#include <dev/dpt/dpt.h>

#define	DPT_VENDOR_ID		0x1044
#define	DPT_DEVICE_ID		0xa400

#define	DPT_PCI_IOADDR		PCIR_BAR(0)		/* I/O Address */
#define	DPT_PCI_MEMADDR		PCIR_BAR(1)		/* Mem I/O Address */

#define	ISA_PRIMARY_WD_ADDRESS	0x1f8

static int	dpt_pci_probe	(device_t);
static int	dpt_pci_attach	(device_t);

static int
dpt_pci_probe (device_t dev)
{
	if ((pci_get_vendor(dev) == DPT_VENDOR_ID) &&
	    (pci_get_device(dev) == DPT_DEVICE_ID)) {
		device_set_desc(dev, "DPT Caching SCSI RAID Controller");
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

static int
dpt_pci_attach (device_t dev)
{
	dpt_softc_t *	dpt;
	int		error = 0;

	u_int32_t	command;

	dpt = device_get_softc(dev);
	dpt->dev = dev;
	dpt_alloc(dev);

	command = pci_read_config(dev, PCIR_COMMAND, /*bytes*/1);

#ifdef DPT_ALLOW_MMIO
	if ((command & PCIM_CMD_MEMEN) != 0) {
		dpt->io_rid = DPT_PCI_MEMADDR;
		dpt->io_type = SYS_RES_MEMORY;
		dpt->io_res = bus_alloc_resource_any(dev, dpt->io_type,
						     &dpt->io_rid, RF_ACTIVE);
	}
#endif
	if (dpt->io_res == NULL && (command &  PCIM_CMD_PORTEN) != 0) {
		dpt->io_rid = DPT_PCI_IOADDR;
		dpt->io_type = SYS_RES_IOPORT;
		dpt->io_res = bus_alloc_resource_any(dev, dpt->io_type,
						     &dpt->io_rid, RF_ACTIVE);
	}

	if (dpt->io_res == NULL) {
		device_printf(dev, "can't allocate register resources\n");
		error = ENOMEM;
		goto bad;
	}
	dpt->io_offset = 0x10;

	dpt->irq_rid = 0;
	dpt->irq_res = bus_alloc_resource_any(dev, SYS_RES_IRQ, &dpt->irq_rid,
					      RF_ACTIVE | RF_SHAREABLE);
	if (dpt->irq_res == NULL) {
		device_printf(dev, "No irq?!\n");
		error = ENOMEM;
		goto bad;
	}

	/* Ensure busmastering is enabled */
	pci_enable_busmaster(dev);

	if (rman_get_start(dpt->io_res) == (ISA_PRIMARY_WD_ADDRESS - 0x10)) {
#ifdef DPT_DEBUG_WARN
		device_printf(dev, "Mapped as an IDE controller.  "
				   "Disabling SCSI setup\n");
#endif
		error = ENXIO;
		goto bad;
	}

	/* Allocate a dmatag representing the capabilities of this attachment */
	if (bus_dma_tag_create(	/* PCI parent */ bus_get_dma_tag(dev),
				/* alignemnt */	1,
				/* boundary  */	0,
				/* lowaddr   */	BUS_SPACE_MAXADDR_32BIT,
				/* highaddr  */	BUS_SPACE_MAXADDR,
				/* filter    */	NULL,
				/* filterarg */	NULL,
				/* maxsize   */	BUS_SPACE_MAXSIZE_32BIT,
				/* nsegments */	~0,
				/* maxsegsz  */	BUS_SPACE_MAXSIZE_32BIT,
				/* flags     */	0,
				/* lockfunc  */ NULL,
				/* lockarg   */ NULL,
				&dpt->parent_dmat) != 0) {
		error = ENXIO;
		goto bad;
	}

	if (dpt_init(dpt) != 0) {
		error = ENXIO;
		goto bad;
	}

	/* Register with the XPT */
	dpt_attach(dpt);

	if (bus_setup_intr(dev, dpt->irq_res, INTR_TYPE_CAM | INTR_ENTROPY |
	    INTR_MPSAFE, NULL, dpt_intr, dpt, &dpt->ih)) {
		device_printf(dev, "Unable to register interrupt handler\n");
		error = ENXIO;
		goto bad;
	}

	return (error);

bad:
	dpt_release_resources(dev);

	dpt_free(dpt);

	return (error);
}

static device_method_t dpt_pci_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,         dpt_pci_probe),
	DEVMETHOD(device_attach,        dpt_pci_attach),
	DEVMETHOD(device_detach,        dpt_detach),

	{ 0, 0 }
};

static driver_t dpt_pci_driver = {
	"dpt",
	dpt_pci_methods,
	sizeof(dpt_softc_t),
};

DRIVER_MODULE(dpt, pci, dpt_pci_driver, dpt_devclass, 0, 0);
MODULE_DEPEND(dpt, pci, 1, 1, 1);
MODULE_DEPEND(dpt, cam, 1, 1, 1);