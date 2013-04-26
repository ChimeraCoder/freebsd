
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
#include <sys/callout.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>
 
#include <isa/isavar.h>
#include <dev/aic/aic6360reg.h>
#include <dev/aic/aicvar.h>
  
struct aic_isa_softc {
	struct	aic_softc sc_aic;
	struct	resource *sc_port;
	struct	resource *sc_irq;
	struct	resource *sc_drq;
	void	*sc_ih;
};

static int aic_isa_alloc_resources(device_t);
static void aic_isa_release_resources(device_t);
static int aic_isa_probe(device_t);
static int aic_isa_attach(device_t);

static u_int aic_isa_ports[] = { 0x340, 0x140 };
#define	AIC_ISA_NUMPORTS (sizeof(aic_isa_ports) / sizeof(aic_isa_ports[0]))
#define	AIC_ISA_PORTSIZE 0x20

static struct isa_pnp_id aic_ids[] = {
	{ 0x15309004, "Adaptec AHA-1530P" },
	{ 0x15209004, "Adaptec AHA-1520P" },
 	{ 0 }
};

static int
aic_isa_alloc_resources(device_t dev)
{
	struct aic_isa_softc *sc = device_get_softc(dev);
	int rid;

	sc->sc_port = sc->sc_irq = sc->sc_drq = NULL;

	rid = 0;
	sc->sc_port = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid,
					0ul, ~0ul, AIC_ISA_PORTSIZE, RF_ACTIVE);
	if (!sc->sc_port) {
		device_printf(dev, "I/O port allocation failed\n");
		return (ENOMEM);
	}

	if (isa_get_irq(dev) != -1) {
		rid = 0;
		sc->sc_irq = bus_alloc_resource_any(dev, SYS_RES_IRQ, &rid,
						    RF_ACTIVE);
		if (!sc->sc_irq) {
			device_printf(dev, "IRQ allocation failed\n");
			aic_isa_release_resources(dev);
			return (ENOMEM);
		}
	}

	if (isa_get_drq(dev) != -1) {
		rid = 0;
		sc->sc_drq = bus_alloc_resource_any(dev, SYS_RES_DRQ, &rid,
						    RF_ACTIVE);
		if (!sc->sc_drq) {
			device_printf(dev, "DRQ allocation failed\n");
			aic_isa_release_resources(dev);
			return (ENOMEM);
		}
	}

	sc->sc_aic.dev = dev;
	sc->sc_aic.res = sc->sc_port;
	mtx_init(&sc->sc_aic.lock, "aic", NULL, MTX_DEF);
	return (0);
}

static void
aic_isa_release_resources(device_t dev)
{
	struct aic_isa_softc *sc = device_get_softc(dev);

	if (sc->sc_port)
		bus_release_resource(dev, SYS_RES_IOPORT, 0, sc->sc_port);
	if (sc->sc_irq)
		bus_release_resource(dev, SYS_RES_IRQ, 0, sc->sc_irq);
	if (sc->sc_drq)
		bus_release_resource(dev, SYS_RES_DRQ, 0, sc->sc_drq);
	sc->sc_port = sc->sc_irq = sc->sc_drq = NULL;
	mtx_destroy(&sc->sc_aic.lock);
}

static int
aic_isa_probe(device_t dev)
{
	struct aic_isa_softc *sc = device_get_softc(dev);
	struct aic_softc *aic = &sc->sc_aic;
	int numports, i;
	u_int port, *ports;
	u_int8_t porta;

	if (ISA_PNP_PROBE(device_get_parent(dev), dev, aic_ids) == ENXIO)
		return (ENXIO);

	port = isa_get_port(dev);
	if (port != -1) {
		ports = &port;
		numports = 1;
	} else {
		ports = aic_isa_ports;
		numports = AIC_ISA_NUMPORTS;
	}

	for (i = 0; i < numports; i++) {
		if (bus_set_resource(dev, SYS_RES_IOPORT, 0, ports[i],
				     AIC_ISA_PORTSIZE))
			continue;
		if (aic_isa_alloc_resources(dev))
			continue;
		if (aic_probe(aic) == 0)
			break;
		aic_isa_release_resources(dev);
	}

	if (i == numports)
		return (ENXIO);

	porta = aic_inb(aic, PORTA);
	aic_isa_release_resources(dev);
	if (isa_get_irq(dev) == -1)
		bus_set_resource(dev, SYS_RES_IRQ, 0, PORTA_IRQ(porta), 1);
	if ((aic->flags & AIC_DMA_ENABLE) && isa_get_drq(dev) == -1)
		bus_set_resource(dev, SYS_RES_DRQ, 0, PORTA_DRQ(porta), 1);
	device_set_desc(dev, "Adaptec 6260/6360 SCSI controller");
	return (0);
}

static int
aic_isa_attach(device_t dev)
{
	struct aic_isa_softc *sc = device_get_softc(dev);
	struct aic_softc *aic = &sc->sc_aic;
	int error;

	error = aic_isa_alloc_resources(dev);
	if (error) {
		device_printf(dev, "resource allocation failed\n");
		return (error);
	}

	error = aic_attach(aic);
	if (error) {
		device_printf(dev, "attach failed\n");
		aic_isa_release_resources(dev);
		return (error);
	}

	error = bus_setup_intr(dev, sc->sc_irq, INTR_TYPE_CAM | INTR_ENTROPY |
	    INTR_MPSAFE, NULL, aic_intr, aic, &sc->sc_ih);
	if (error) {
		device_printf(dev, "failed to register interrupt handler\n");
		aic_isa_release_resources(dev);
		return (error);
	}
	return (0);
}

static int
aic_isa_detach(device_t dev)
{
	struct aic_isa_softc *sc = device_get_softc(dev);
	struct aic_softc *aic = &sc->sc_aic;
	int error;

	error = aic_detach(aic);
	if (error) {
		device_printf(dev, "detach failed\n");
		return (error);
	}

	error = bus_teardown_intr(dev, sc->sc_irq, sc->sc_ih);
	if (error) {
		device_printf(dev, "failed to unregister interrupt handler\n");
	}

	aic_isa_release_resources(dev);
	return (0);
}

static device_method_t aic_isa_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		aic_isa_probe),
	DEVMETHOD(device_attach,	aic_isa_attach),
	DEVMETHOD(device_detach,	aic_isa_detach),
	{ 0, 0 }
};

static driver_t aic_isa_driver = {
	"aic",
	aic_isa_methods, sizeof(struct aic_isa_softc),
};

extern devclass_t aic_devclass;

MODULE_DEPEND(aic, cam, 1,1,1);
DRIVER_MODULE(aic, isa, aic_isa_driver, aic_devclass, 0, 0);