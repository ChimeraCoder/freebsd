
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

#include "opt_ddb.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/malloc.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <sys/rman.h>

#include <machine/resource.h>

#include <dev/ofw/openfirm.h>

#include <powerpc/powermac/maciovar.h>

struct pswitch_softc {
	int	sc_irqrid;
	struct	resource *sc_irq;
	void	*sc_ih;
};

static int	pswitch_probe(device_t);
static int	pswitch_attach(device_t);

static int	pswitch_intr(void *);

static device_method_t pswitch_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		pswitch_probe),
	DEVMETHOD(device_attach,	pswitch_attach),

	{ 0, 0 }
};

static driver_t pswitch_driver = {
	"pswitch",
	pswitch_methods,
	sizeof(struct pswitch_softc)
};

static devclass_t pswitch_devclass;

DRIVER_MODULE(pswitch, macio, pswitch_driver, pswitch_devclass, 0, 0);

static int
pswitch_probe(device_t dev)
{
	char	*type = macio_get_devtype(dev);

	if (strcmp(type, "gpio") != 0)
		return (ENXIO);

	device_set_desc(dev, "GPIO Programmer's Switch");
	return (0);
}

static int
pswitch_attach(device_t dev)
{
	struct		pswitch_softc *sc;
	phandle_t	node, child;
	char		type[32];
	u_int		irq[2];

	sc = device_get_softc(dev);
	node = macio_get_node(dev);

	for (child = OF_child(node); child != 0; child = OF_peer(child)) {
		if (OF_getprop(child, "device_type", type, 32) == -1)
			continue;

		if (strcmp(type, "programmer-switch") == 0)
			break;
	}

	if (child == 0) {
		device_printf(dev, "could not find correct node\n");
		return (ENXIO);
	}

	if (OF_getprop(child, "interrupts", irq, sizeof(irq)) == -1) {
		device_printf(dev, "could not get interrupt\n");
		return (ENXIO);
	}

	sc->sc_irqrid = 0;
	sc->sc_irq = bus_alloc_resource(dev, SYS_RES_IRQ, &sc->sc_irqrid,
	    irq[0], irq[0], 1, RF_ACTIVE);
	if (sc->sc_irq == NULL) {
		device_printf(dev, "could not allocate interrupt\n");
		return (ENXIO);
	}

	if (bus_setup_intr(dev, sc->sc_irq, INTR_TYPE_MISC,
	    pswitch_intr, NULL, dev, &sc->sc_ih) != 0) {
		device_printf(dev, "could not setup interrupt\n");
		bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irqrid,
		    sc->sc_irq);
		return (ENXIO);
	}

	return (0);
}

static int
pswitch_intr(void *arg)
{
	device_t	dev;

	dev = (device_t)arg;

	kdb_enter(KDB_WHY_POWERPC, device_get_nameunit(dev));
	return (FILTER_HANDLED);
}