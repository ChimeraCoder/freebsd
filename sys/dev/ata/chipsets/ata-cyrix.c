
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
static int ata_cyrix_chipinit(device_t dev);
static int ata_cyrix_ch_attach(device_t dev);
static int ata_cyrix_setmode(device_t dev, int target, int mode);

/*
 * Cyrix chipset support functions
 */
static int
ata_cyrix_probe(device_t dev)
{
    struct ata_pci_controller *ctlr = device_get_softc(dev);

    if (pci_get_devid(dev) == ATA_CYRIX_5530) {
	device_set_desc(dev, "Cyrix 5530 ATA33 controller");
	ctlr->chipinit = ata_cyrix_chipinit;
	return (BUS_PROBE_DEFAULT);
    }
    return ENXIO;
}

static int
ata_cyrix_chipinit(device_t dev)
{
    struct ata_pci_controller *ctlr = device_get_softc(dev);

    if (ata_setup_interrupt(dev, ata_generic_intr))
	return ENXIO;
    ctlr->ch_attach = ata_cyrix_ch_attach;
    ctlr->setmode = ata_cyrix_setmode;
    return 0;
}

static int
ata_cyrix_ch_attach(device_t dev)
{
	struct ata_channel *ch = device_get_softc(dev);
 
	ch->dma.alignment = 16;
	ch->dma.max_iosize = 64 * DEV_BSIZE;
	return (ata_pci_ch_attach(dev));
}

static int
ata_cyrix_setmode(device_t dev, int target, int mode)
{
	struct ata_pci_controller *ctlr = device_get_softc(device_get_parent(dev));
	struct ata_channel *ch = device_get_softc(dev);
	int devno = (ch->unit << 1) + target;
	int piomode;
	static const uint32_t piotiming[] = 
	    { 0x00009172, 0x00012171, 0x00020080, 0x00032010, 0x00040010 };
	static const uint32_t dmatiming[] =
	    { 0x00077771, 0x00012121, 0x00002020 };
	static const uint32_t udmatiming[] =
	    { 0x00921250, 0x00911140, 0x00911030 };

	mode = min(mode, ATA_UDMA2);
	/* dont try to set the mode if we dont have the resource */
	if (ctlr->r_res1) {
		if (mode >= ATA_UDMA0) {
			/* Set UDMA timings, and PIO4. */
			ATA_OUTL(ch->r_io[ATA_BMCMD_PORT].res,
			    0x24 + (devno << 3), udmatiming[mode & ATA_MODE_MASK]);
			piomode = ATA_PIO4;
		} else if (mode >= ATA_WDMA0) {
			/* Set WDMA timings, and respective PIO mode. */
			ATA_OUTL(ch->r_io[ATA_BMCMD_PORT].res,
			    0x24 + (devno << 3), dmatiming[mode & ATA_MODE_MASK]);
		        piomode = (mode == ATA_WDMA0) ? ATA_PIO0 :
			    (mode == ATA_WDMA1) ? ATA_PIO3 : ATA_PIO4;
		} else
			piomode = mode;
		/* Set PIO mode calculated above. */
		ATA_OUTL(ch->r_io[ATA_BMCMD_PORT].res,
		    0x20 + (devno << 3), piotiming[ata_mode2idx(piomode)]);
	}
	return (mode);
}

ATA_DECLARE_DRIVER(ata_cyrix);