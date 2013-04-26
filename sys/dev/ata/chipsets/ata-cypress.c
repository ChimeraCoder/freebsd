
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
static int ata_cypress_chipinit(device_t dev);
static int ata_cypress_setmode(device_t dev, int target, int mode);


/*
 * Cypress chipset support functions
 */
static int
ata_cypress_probe(device_t dev)
{
    struct ata_pci_controller *ctlr = device_get_softc(dev);

    /*
     * the Cypress chip is a mess, it contains two ATA functions, but
     * both channels are visible on the first one.
     * simply ignore the second function for now, as the right
     * solution (ignoring the second channel on the first function)
     * doesn't work with the crappy ATA interrupt setup on the alpha.
     */
    if (pci_get_devid(dev) == ATA_CYPRESS_82C693 &&
	pci_get_function(dev) == 1 &&
	pci_get_subclass(dev) == PCIS_STORAGE_IDE) {
	device_set_desc(dev, "Cypress 82C693 ATA controller");
	ctlr->chipinit = ata_cypress_chipinit;
	return (BUS_PROBE_DEFAULT);
    }
    return ENXIO;
}

static int
ata_cypress_chipinit(device_t dev)
{
    struct ata_pci_controller *ctlr = device_get_softc(dev);

    if (ata_setup_interrupt(dev, ata_generic_intr))
	return ENXIO;

    ctlr->setmode = ata_cypress_setmode;
    return 0;
}

static int
ata_cypress_setmode(device_t dev, int target, int mode)
{
	device_t parent = device_get_parent(dev);
	struct ata_channel *ch = device_get_softc(dev);

	mode = min(mode, ATA_WDMA2);

	/* XXX SOS missing WDMA0+1 + PIO modes */
	if (mode == ATA_WDMA2) { 
		pci_write_config(parent, ch->unit ? 0x4e : 0x4c, 0x2020, 2);
	}
	/* we could set PIO mode timings, but we assume the BIOS did that */
	return (mode);
}

ATA_DECLARE_DRIVER(ata_cypress);