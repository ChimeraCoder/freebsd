
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <sys/module.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <isa/isavar.h>

#include <dev/mca/mca_busreg.h>
#include <dev/mca/mca_busvar.h>

#include <dev/aha/ahareg.h>

static struct mca_ident aha_mca_devs[] = {
	{ 0x0f1f, "Adaptec AHA-1640 SCSI Adapter" },
	{ 0, NULL },
};

#define AHA_MCA_IOPORT_POS		MCA_ADP_POS(MCA_POS1)
# define AHA_MCA_IOPORT_MASK1		0x07
# define AHA_MCA_IOPORT_MASK2		0xc0
# define AHA_MCA_IOPORT_SIZE		0x03
# define AHA_MCA_IOPORT(pos)		(0x30 + \
					(((uint32_t)pos & \
						AHA_MCA_IOPORT_MASK1) << 8) + \
					(((uint32_t)pos & \
						AHA_MCA_IOPORT_MASK2) >> 4))

#define AHA_MCA_DRQ_POS			MCA_ADP_POS(MCA_POS3)
# define AHA_MCA_DRQ_MASK		0x0f
# define AHA_MCA_DRQ(pos)		(pos & AHA_MCA_DRQ_MASK)

#define AHA_MCA_IRQ_POS			MCA_ADP_POS(MCA_POS2)
# define AHA_MCA_IRQ_MASK		0x07
# define AHA_MCA_IRQ(pos)		((pos & AHA_MCA_IRQ_MASK) + 8)

/*
 * Not needed as the board knows its config
 * internally and the ID will be fetched 
 * via AOP_INQUIRE_SETUP_INFO command.
 */
#define AHA_MCA_SCSIID_POS		MCA_ADP_POS(MCA_POS2)
#define AHA_MCA_SCSIID_MASK		0xe0
#define AHA_MCA_SCSIID(pos)		((pos & AHA_MCA_SCSIID_MASK) >> 5)

static int
aha_mca_probe (device_t dev)
{
	const char *	desc;
	mca_id_t	id = mca_get_id(dev);
	uint32_t	iobase = 0;
	uint32_t	iosize = 0;
	uint8_t		drq = 0;
	uint8_t		irq = 0;
	uint8_t		pos;

	desc = mca_match_id(id, aha_mca_devs);
	if (!desc)
		return (ENXIO);
	device_set_desc(dev, desc);

	pos = mca_pos_read(dev, AHA_MCA_IOPORT_POS);
	iobase = AHA_MCA_IOPORT(pos);
	iosize = AHA_MCA_IOPORT_SIZE;

	pos = mca_pos_read(dev, AHA_MCA_DRQ_POS);
	drq = AHA_MCA_DRQ(pos);

	pos = mca_pos_read(dev, AHA_MCA_IRQ_POS);
	irq = AHA_MCA_IRQ(pos);

	mca_add_iospace(dev, iobase, iosize);
	mca_add_drq(dev, drq);
	mca_add_irq(dev, irq);

	return (0);
}

static int
aha_mca_attach (device_t dev)
{
	struct aha_softc *	sc = device_get_softc(dev);
	int			error = ENOMEM;

	sc->portrid = 0;
	sc->port = bus_alloc_resource_any(dev, SYS_RES_IOPORT, &sc->portrid,
	    RF_ACTIVE);
	if (sc->port == NULL) {
		device_printf(dev, "No I/O space?!\n");
		goto bad;
	}

	sc->irqrid = 0;
	sc->irq = bus_alloc_resource_any(dev, SYS_RES_IRQ, &sc->irqrid,
	    RF_ACTIVE);
	if (sc->irq == NULL) {
		device_printf(dev, "No IRQ?!\n");
		goto bad;
	}

	sc->drqrid = 0;
	sc->drq = bus_alloc_resource_any(dev, SYS_RES_DRQ, &sc->drqrid,
	    RF_ACTIVE);
	if (sc->drq == NULL) {
		device_printf(dev, "No DRQ?!\n");
		goto bad;
	}

	aha_alloc(sc);
	error = aha_probe(sc);
	if (error) {
		device_printf(dev, "aha_probe() failed!\n");
		goto bad;
	}

	error = aha_fetch_adapter_info(sc);
	if (error) {
		device_printf(dev, "aha_fetch_adapter_info() failed!\n");
		goto bad;
	}

	isa_dmacascade(rman_get_start(sc->drq));

	error = bus_dma_tag_create(
				/* parent	*/ bus_get_dma_tag(dev),
				/* alignemnt	*/ 1,
				/* boundary	*/ 0,
				/* lowaddr	*/ BUS_SPACE_MAXADDR_24BIT,
				/* highaddr	*/ BUS_SPACE_MAXADDR,
				/* filter	*/ NULL,
				/* filterarg	*/ NULL,
				/* maxsize	*/ BUS_SPACE_MAXSIZE_24BIT,
				/* nsegments	*/ ~0,
				/* maxsegsz	*/ BUS_SPACE_MAXSIZE_24BIT,
				/* flags	*/ 0,
				/* lockfunc	*/ NULL,
				/* lockarg	*/ NULL,
				&sc->parent_dmat);
	if (error) {
		device_printf(dev, "bus_dma_tag_create() failed!\n");
		goto bad;
	}			      

	error = aha_init(sc);
	if (error) {
		device_printf(dev, "aha_init() failed\n");
		goto bad;
	}

	error = aha_attach(sc);
	if (error) {
		device_printf(dev, "aha_attach() failed\n");
		goto bad;
	}

	error = bus_setup_intr(dev, sc->irq, INTR_TYPE_CAM | INTR_ENTROPY |
	    INTR_MPSAFE, NULL, aha_intr, sc, &sc->ih);
	if (error) {
		device_printf(dev, "Unable to register interrupt handler\n");
		aha_detach(sc);
		goto bad;
	}

	return (0);

bad:
	aha_free(sc);
	bus_free_resource(dev, SYS_RES_IOPORT, sc->port);
	bus_free_resource(dev, SYS_RES_IRQ, sc->irq);
	bus_free_resource(dev, SYS_RES_DRQ, sc->drq);
	return (error);
}

static device_method_t aha_mca_methods[] = {
	DEVMETHOD(device_probe,         aha_mca_probe),
	DEVMETHOD(device_attach,        aha_mca_attach),

	{ 0, 0 }
};

static driver_t aha_mca_driver = {
	"aha",
	aha_mca_methods,
	1,
/*
	sizeof(struct aha_softc *),
 */
};

static devclass_t aha_devclass;

DRIVER_MODULE(aha, mca, aha_mca_driver, aha_devclass, 0, 0);
MODULE_DEPEND(aha, mca, 1, 1, 1);