
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
static int ata_netcell_chipinit(device_t dev);
static int ata_netcell_ch_attach(device_t dev);
static int ata_netcell_setmode(device_t dev, int target, int mode);

/*
 * NetCell chipset support functions
 */
static int
ata_netcell_probe(device_t dev)
{
    struct ata_pci_controller *ctlr = device_get_softc(dev);

    if (pci_get_devid(dev) == ATA_NETCELL_SR) {
	device_set_desc(dev, "Netcell SyncRAID SR3000/5000 RAID Controller");
	ctlr->chipinit = ata_netcell_chipinit;
	return (BUS_PROBE_DEFAULT);
    }
    return ENXIO;
}

static int
ata_netcell_chipinit(device_t dev)
{
    struct ata_pci_controller *ctlr = device_get_softc(dev);

    if (ata_setup_interrupt(dev, ata_generic_intr))
        return ENXIO;

    ctlr->ch_attach = ata_netcell_ch_attach;
    ctlr->setmode = ata_netcell_setmode;
    return 0;
}

static int
ata_netcell_ch_attach(device_t dev)
{
    struct ata_channel *ch = device_get_softc(dev);
 
    /* setup the usual register normal pci style */
    if (ata_pci_ch_attach(dev))
	return ENXIO;
 
    /* the NetCell only supports 16 bit PIO transfers */
    ch->flags |= ATA_USE_16BIT;
    /* It is a hardware RAID without cable. */
    ch->flags |= ATA_CHECKS_CABLE;
    return 0;
}

static int
ata_netcell_setmode(device_t dev, int target, int mode)
{

	return (min(mode, ATA_UDMA6));
}

ATA_DECLARE_DRIVER(ata_netcell);