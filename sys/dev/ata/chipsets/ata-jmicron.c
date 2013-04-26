
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
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/ata.h>
#include <sys/bus.h>
#include <sys/endian.h>
#include <sys/malloc.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/sema.h>
#include <sys/taskqueue.h>
#include <vm/uma.h>
#include <machine/stdarg.h>
#include <machine/resource.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/ata/ata-all.h>
#include <dev/ata/ata-pci.h>
#include <ata_if.h>

/* local prototypes */
static int ata_jmicron_chipinit(device_t dev);
static int ata_jmicron_ch_attach(device_t dev);
static int ata_jmicron_setmode(device_t dev, int target, int mode);

/*
 * JMicron chipset support functions
 */
static int
ata_jmicron_probe(device_t dev)
{
    struct ata_pci_controller *ctlr = device_get_softc(dev);
    const struct ata_chip_id *idx;
    static const struct ata_chip_id ids[] =
    {{ ATA_JMB360, 0, 1, 0, ATA_SA300, "JMB360" },
     { ATA_JMB361, 0, 1, 1, ATA_UDMA6, "JMB361" },
     { ATA_JMB362, 0, 2, 0, ATA_SA300, "JMB362" },
     { ATA_JMB363, 0, 2, 1, ATA_UDMA6, "JMB363" },
     { ATA_JMB365, 0, 1, 2, ATA_UDMA6, "JMB365" },
     { ATA_JMB366, 0, 2, 2, ATA_UDMA6, "JMB366" },
     { ATA_JMB368, 0, 0, 1, ATA_UDMA6, "JMB368" },
     { 0, 0, 0, 0, 0, 0}};
    char buffer[64];

    if (pci_get_vendor(dev) != ATA_JMICRON_ID)
	return ENXIO;

    if (!(idx = ata_match_chip(dev, ids)))
        return ENXIO;

    sprintf(buffer, "JMicron %s %s controller",
	idx->text, ata_mode2str(idx->max_dma));
    device_set_desc_copy(dev, buffer);
    ctlr->chip = idx;
    ctlr->chipinit = ata_jmicron_chipinit;
    return (BUS_PROBE_DEFAULT);
}

static int
ata_jmicron_chipinit(device_t dev)
{
    struct ata_pci_controller *ctlr = device_get_softc(dev);
    device_t child;

    if (ata_setup_interrupt(dev, ata_generic_intr))
	return ENXIO;

    /* do we have multiple PCI functions ? */
    if (pci_read_config(dev, 0xdf, 1) & 0x40) {
	/* are we on the AHCI part ? */
	if (ata_ahci_chipinit(dev) != ENXIO)
	    return 0;

	/* otherwise we are on the PATA part */
	ctlr->ch_attach = ata_jmicron_ch_attach;
	ctlr->ch_detach = ata_pci_ch_detach;
	ctlr->reset = ata_generic_reset;
	ctlr->setmode = ata_jmicron_setmode;
	ctlr->channels = ctlr->chip->cfg2;
    }
    else {
	/* set controller configuration to a combined setup we support */
	pci_write_config(dev, 0x40, 0x80c0a131, 4);
	pci_write_config(dev, 0x80, 0x01200000, 4);
	/* Create AHCI subdevice if AHCI part present. */
	if (ctlr->chip->cfg1) {
	    	child = device_add_child(dev, NULL, -1);
		if (child != NULL) {
		    device_set_ivars(child, (void *)(intptr_t)-1);
		    bus_generic_attach(dev);
		}
	}
	ctlr->ch_attach = ata_jmicron_ch_attach;
	ctlr->ch_detach = ata_pci_ch_detach;
	ctlr->reset = ata_generic_reset;
	ctlr->setmode = ata_jmicron_setmode;
	ctlr->channels = ctlr->chip->cfg2;
    }
    return 0;
}

static int
ata_jmicron_ch_attach(device_t dev)
{
	struct ata_channel *ch = device_get_softc(dev);
	int error;
 
	error = ata_pci_ch_attach(dev);
	ch->flags |= ATA_CHECKS_CABLE;
	return (error);
}

static int
ata_jmicron_setmode(device_t dev, int target, int mode)
{
	struct ata_pci_controller *ctlr = device_get_softc(device_get_parent(dev));

	mode = min(mode, ctlr->chip->max_dma);
	/* check for 80pin cable present */
	if (ata_dma_check_80pin && mode > ATA_UDMA2 &&
	    pci_read_config(dev, 0x40, 1) & 0x08) {
		ata_print_cable(dev, "controller");
		mode = ATA_UDMA2;
	}
	/* Nothing to do to setup mode, the controller snoop SET_FEATURE cmd. */
	return (mode);
}

ATA_DECLARE_DRIVER(ata_jmicron);
MODULE_DEPEND(ata_jmicron, ata_ahci, 1, 1, 1);