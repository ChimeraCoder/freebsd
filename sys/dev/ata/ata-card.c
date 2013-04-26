
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
#include <sys/ata.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/malloc.h>
#include <sys/sema.h>
#include <sys/taskqueue.h>
#include <vm/uma.h>
#include <machine/stdarg.h>
#include <machine/resource.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <dev/ata/ata-all.h>
#include <dev/pccard/pccard_cis.h>
#include <dev/pccard/pccardreg.h>
#include <dev/pccard/pccardvar.h>
#include <ata_if.h>

#include "pccarddevs.h"

static const struct pccard_product ata_pccard_products[] = {
	PCMCIA_CARD(FREECOM, PCCARDIDE),
	PCMCIA_CARD(EXP, EXPMULTIMEDIA),
	PCMCIA_CARD(IODATA3, CBIDE2),
	PCMCIA_CARD(OEM2, CDROM1),
	PCMCIA_CARD(OEM2, IDE),
	PCMCIA_CARD(PANASONIC, KXLC005),
	PCMCIA_CARD(TEAC, IDECARDII),
	{NULL}
};

static int
ata_pccard_probe(device_t dev)
{
    const struct pccard_product *pp;
    u_int32_t fcn = PCCARD_FUNCTION_UNSPEC;
    int error = 0;

    if ((error = pccard_get_function(dev, &fcn)))
	return error;

    /* if it says its a disk we should register it */
    if (fcn == PCCARD_FUNCTION_DISK)
	return 0;

    /* match other devices here, primarily cdrom/dvd rom */
    if ((pp = pccard_product_lookup(dev, ata_pccard_products,
				    sizeof(ata_pccard_products[0]), NULL))) {
	if (pp->pp_name)
	    device_set_desc(dev, pp->pp_name);
	return 0;
    }
    return ENXIO;
}

static int
ata_pccard_attach(device_t dev)
{
    struct ata_channel *ch = device_get_softc(dev);
    struct resource *io, *ctlio;
    int i, rid, err;
    uint16_t funce;

    if (ch->attached)
	return (0);
    ch->attached = 1;

    /* allocate the io range to get start and length */
    rid = ATA_IOADDR_RID;
    if (!(io = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid, 0, ~0,
				  ATA_IOSIZE, RF_ACTIVE)))
	return (ENXIO);

    /* setup the resource vectors */
    for (i = ATA_DATA; i <= ATA_COMMAND; i++) {
	ch->r_io[i].res = io;
	ch->r_io[i].offset = i;
    }
    ch->r_io[ATA_IDX_ADDR].res = io;

    /*
     * if we got more than the default ATA_IOSIZE ports, this is a device
     * where ctlio is located at offset 14 into "normal" io space.
     */
    if (rman_get_size(io) > ATA_IOSIZE) {
	ch->r_io[ATA_CONTROL].res = io;
	ch->r_io[ATA_CONTROL].offset = 14;
    }
    else {
	rid = ATA_CTLADDR_RID;
	if (!(ctlio = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid, 0, ~0,
					 ATA_CTLIOSIZE, RF_ACTIVE))) {
	    bus_release_resource(dev, SYS_RES_IOPORT, ATA_IOADDR_RID, io);
	    for (i = ATA_DATA; i < ATA_MAX_RES; i++)
		ch->r_io[i].res = NULL;
	    return (ENXIO);
	}
	ch->r_io[ATA_CONTROL].res = ctlio;
	ch->r_io[ATA_CONTROL].offset = 0;
    }
    ata_default_registers(dev);

    /* initialize softc for this channel */
    ch->unit = 0;
    ch->flags |= ATA_USE_16BIT;
    funce = 0;		/* Default to sane setting of FUNCE */
    pccard_get_funce_disk(dev, &funce); 
    if (!(funce & PFD_I_D))
        ch-> flags |= ATA_NO_SLAVE;
    ata_generic_hw(dev);
    err = ata_probe(dev);
    if (err)
	return (err);
    return (ata_attach(dev));
}

static int
ata_pccard_detach(device_t dev)
{
    struct ata_channel *ch = device_get_softc(dev);
    int i;

    if (!ch->attached)
	return (0);
    ch->attached = 0;

    ata_detach(dev);
    if (ch->r_io[ATA_CONTROL].res != ch->r_io[ATA_DATA].res)
	bus_release_resource(dev, SYS_RES_IOPORT, ATA_CTLADDR_RID,
			     ch->r_io[ATA_CONTROL].res);
    bus_release_resource(dev, SYS_RES_IOPORT, ATA_IOADDR_RID,
			 ch->r_io[ATA_DATA].res);
    for (i = ATA_DATA; i < ATA_MAX_RES; i++)
	ch->r_io[i].res = NULL;
    return 0;
}

static device_method_t ata_pccard_methods[] = {
    /* device interface */
    DEVMETHOD(device_probe,             ata_pccard_probe),
    DEVMETHOD(device_attach,            ata_pccard_attach),
    DEVMETHOD(device_detach,            ata_pccard_detach),

    DEVMETHOD_END
};

static driver_t ata_pccard_driver = {
    "ata",
    ata_pccard_methods,
    sizeof(struct ata_channel),
};

DRIVER_MODULE(ata, pccard, ata_pccard_driver, ata_devclass, NULL, NULL);
MODULE_DEPEND(ata, ata, 1, 1, 1);