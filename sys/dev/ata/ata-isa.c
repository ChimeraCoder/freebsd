
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
#include <isa/isavar.h>
#include <dev/ata/ata-all.h>
#include <ata_if.h>

/* local vars */
static struct isa_pnp_id ata_ids[] = {
    {0x0006d041,        "Generic ESDI/IDE/ATA controller"},     /* PNP0600 */
    {0x0106d041,        "Plus Hardcard II"},                    /* PNP0601 */
    {0x0206d041,        "Plus Hardcard IIXL/EZ"},               /* PNP0602 */
    {0x0306d041,        "Generic ATA"},                         /* PNP0603 */
								/* PNP0680 */
    {0x8006d041,        "Standard bus mastering IDE hard disk controller"},
    {0}
};

static int
ata_isa_probe(device_t dev)
{
    struct resource *io = NULL, *ctlio = NULL;
    u_long tmp;
    int rid;

    /* check isapnp ids */
    if (ISA_PNP_PROBE(device_get_parent(dev), dev, ata_ids) == ENXIO)
	return ENXIO;

    /* allocate the io port range */
    rid = ATA_IOADDR_RID;
    if (!(io = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid, 0, ~0,
				  ATA_IOSIZE, RF_ACTIVE)))
	return ENXIO;

    /* set the altport range */
    if (bus_get_resource(dev, SYS_RES_IOPORT, ATA_CTLADDR_RID, &tmp, &tmp)) {
	bus_set_resource(dev, SYS_RES_IOPORT, ATA_CTLADDR_RID,
			 rman_get_start(io) + ATA_CTLOFFSET, ATA_CTLIOSIZE);
    }

    /* allocate the altport range */
    rid = ATA_CTLADDR_RID; 
    if (!(ctlio = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid, 0, ~0,
				     ATA_CTLIOSIZE, RF_ACTIVE))) {
	bus_release_resource(dev, SYS_RES_IOPORT, ATA_IOADDR_RID, io);
	return ENXIO;
    }

    /* Release resources to reallocate on attach. */
    bus_release_resource(dev, SYS_RES_IOPORT, ATA_CTLADDR_RID, ctlio);
    bus_release_resource(dev, SYS_RES_IOPORT, ATA_IOADDR_RID, io);

    device_set_desc(dev, "ATA channel");
    return (ata_probe(dev));
}

static int
ata_isa_attach(device_t dev)
{
    struct ata_channel *ch = device_get_softc(dev);
    struct resource *io = NULL, *ctlio = NULL;
    u_long tmp;
    int i, rid;

    if (ch->attached)
	return (0);
    ch->attached = 1;

    /* allocate the io port range */
    rid = ATA_IOADDR_RID;
    if (!(io = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid, 0, ~0,
				  ATA_IOSIZE, RF_ACTIVE)))
	return ENXIO;

    /* set the altport range */
    if (bus_get_resource(dev, SYS_RES_IOPORT, ATA_CTLADDR_RID, &tmp, &tmp)) {
	bus_set_resource(dev, SYS_RES_IOPORT, ATA_CTLADDR_RID,
			 rman_get_start(io) + ATA_CTLOFFSET, ATA_CTLIOSIZE);
    }

    /* allocate the altport range */
    rid = ATA_CTLADDR_RID; 
    if (!(ctlio = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid, 0, ~0,
				     ATA_CTLIOSIZE, RF_ACTIVE))) {
	bus_release_resource(dev, SYS_RES_IOPORT, ATA_IOADDR_RID, io);
	return ENXIO;
    }

    /* setup the resource vectors */
    for (i = ATA_DATA; i <= ATA_COMMAND; i++) {
	ch->r_io[i].res = io;
	ch->r_io[i].offset = i;
    }
    ch->r_io[ATA_CONTROL].res = ctlio;
    ch->r_io[ATA_CONTROL].offset = 0;
    ch->r_io[ATA_IDX_ADDR].res = io;
    ata_default_registers(dev);
 
    /* initialize softc for this channel */
    ch->unit = 0;
    ch->flags |= ATA_USE_16BIT;
    ata_generic_hw(dev);
    return ata_attach(dev);
}

static int
ata_isa_detach(device_t dev)
{
    struct ata_channel *ch = device_get_softc(dev);
    int error;

    if (!ch->attached)
	return (0);
    ch->attached = 0;

    error = ata_detach(dev);

    bus_release_resource(dev, SYS_RES_IOPORT, ATA_CTLADDR_RID,
	ch->r_io[ATA_CONTROL].res);
    bus_release_resource(dev, SYS_RES_IOPORT, ATA_IOADDR_RID,
	ch->r_io[ATA_IDX_ADDR].res);
    return (error);
}

static int
ata_isa_suspend(device_t dev)
{
    struct ata_channel *ch = device_get_softc(dev);

    if (!ch->attached)
	return (0);

    return ata_suspend(dev);
}

static int
ata_isa_resume(device_t dev)
{
    struct ata_channel *ch = device_get_softc(dev);

    if (!ch->attached)
	return (0);

    return ata_resume(dev);
}


static device_method_t ata_isa_methods[] = {
    /* device interface */
    DEVMETHOD(device_probe,     ata_isa_probe),
    DEVMETHOD(device_attach,    ata_isa_attach),
    DEVMETHOD(device_detach,    ata_isa_detach),
    DEVMETHOD(device_suspend,   ata_isa_suspend),
    DEVMETHOD(device_resume,    ata_isa_resume),

    DEVMETHOD_END
};

static driver_t ata_isa_driver = {
    "ata",
    ata_isa_methods,
    sizeof(struct ata_channel),
};

DRIVER_MODULE(ata, isa, ata_isa_driver, ata_devclass, NULL, NULL);
MODULE_DEPEND(ata, ata, 1, 1, 1);