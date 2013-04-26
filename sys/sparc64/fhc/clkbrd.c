
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
#include <sys/resource.h>
#include <sys/rman.h>

#include <dev/led/led.h>
#include <dev/ofw/ofw_bus.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <sparc64/fhc/clkbrdreg.h>

#define	CLKBRD_NREG	3

#define	CLKBRD_CF	0
#define	CLKBRD_CLK	1
#define	CLKBRD_CLKVER	2

struct clkbrd_softc {
	struct device		*sc_dev;
	struct resource		*sc_res[CLKBRD_NREG];
	int			sc_rid[CLKBRD_NREG];
	bus_space_tag_t		sc_bt[CLKBRD_NREG];
	bus_space_handle_t	sc_bh[CLKBRD_NREG];
	uint8_t			sc_clk_ctrl;
	struct cdev		*sc_led_dev;
	int			sc_flags;
#define	CLKBRD_HAS_CLKVER	(1 << 0)
};

static devclass_t clkbrd_devclass;

static device_probe_t clkbrd_probe;
static device_attach_t clkbrd_attach;
static device_detach_t clkbrd_detach;

static void clkbrd_free_resources(struct clkbrd_softc *);
static void clkbrd_led_func(void *, int);

static device_method_t clkbrd_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		clkbrd_probe),
	DEVMETHOD(device_attach,	clkbrd_attach),
	DEVMETHOD(device_detach,	clkbrd_detach),

        { 0, 0 }
};

static driver_t clkbrd_driver = {
        "clkbrd",
        clkbrd_methods,
        sizeof(struct clkbrd_softc),
};

DRIVER_MODULE(clkbrd, fhc, clkbrd_driver, clkbrd_devclass, 0, 0);

static int
clkbrd_probe(device_t dev)
{

	if (strcmp(ofw_bus_get_name(dev), "clock-board") == 0) {
		device_set_desc(dev, "Clock Board");
		return (0);
	}
	return (ENXIO);
}

static int
clkbrd_attach(device_t dev)
{
	struct clkbrd_softc *sc;
	int i, slots;
	uint8_t r;

	sc = device_get_softc(dev);
	sc->sc_dev = dev;

	for (i = CLKBRD_CF; i <= CLKBRD_CLKVER; i++) {
		sc->sc_rid[i] = i;
		sc->sc_res[i] = bus_alloc_resource_any(sc->sc_dev,
		    SYS_RES_MEMORY, &sc->sc_rid[i], RF_ACTIVE);
		if (sc->sc_res[i] == NULL) {
			if (i != CLKBRD_CLKVER) {
				device_printf(sc->sc_dev,
				    "could not allocate resource %d\n", i);
				goto fail;
			}
			continue;
		}
		sc->sc_bt[i] = rman_get_bustag(sc->sc_res[i]);
		sc->sc_bh[i] = rman_get_bushandle(sc->sc_res[i]);
		if (i == CLKBRD_CLKVER)
			sc->sc_flags |= CLKBRD_HAS_CLKVER;
	}

	slots = 4;
	r = bus_space_read_1(sc->sc_bt[CLKBRD_CLK], sc->sc_bh[CLKBRD_CLK],
	    CLK_STS1);
	switch (r & CLK_STS1_SLOTS_MASK) {
	case CLK_STS1_SLOTS_16:
		slots = 16;
		break;
	case CLK_STS1_SLOTS_8:
		slots = 8;
		break;
	case CLK_STS1_SLOTS_4:
		if (sc->sc_flags & CLKBRD_HAS_CLKVER) {
			r = bus_space_read_1(sc->sc_bt[CLKBRD_CLKVER],
			    sc->sc_bh[CLKBRD_CLKVER], CLKVER_SLOTS);
			if (r != 0 &&
			    (r & CLKVER_SLOTS_MASK) == CLKVER_SLOTS_PLUS)
				slots = 5;
		}
	}

	device_printf(sc->sc_dev, "Sun Enterprise Exx00 machine: %d slots\n",
	    slots);

	sc->sc_clk_ctrl = bus_space_read_1(sc->sc_bt[CLKBRD_CLK],
	    sc->sc_bh[CLKBRD_CLK], CLK_CTRL);
	sc->sc_led_dev = led_create(clkbrd_led_func, sc, "clockboard");

	return (0);

 fail:
	clkbrd_free_resources(sc);

	return (ENXIO);
}

static int
clkbrd_detach(device_t dev)
{
	struct clkbrd_softc *sc;

	sc = device_get_softc(dev);

	led_destroy(sc->sc_led_dev);
	bus_space_write_1(sc->sc_bt[CLKBRD_CLK], sc->sc_bh[CLKBRD_CLK],
	    CLK_CTRL, sc->sc_clk_ctrl);
	bus_space_read_1(sc->sc_bt[CLKBRD_CLK], sc->sc_bh[CLKBRD_CLK],
	    CLK_CTRL);
	clkbrd_free_resources(sc);

	return (0);
}

static void
clkbrd_free_resources(struct clkbrd_softc *sc)
{
	int i;

	for (i = CLKBRD_CF; i <= CLKBRD_CLKVER; i++)
		if (sc->sc_res[i] != NULL)
			bus_release_resource(sc->sc_dev, SYS_RES_MEMORY,
			    sc->sc_rid[i], sc->sc_res[i]);
}

static void
clkbrd_led_func(void *arg, int onoff)
{
	struct clkbrd_softc *sc;
	uint8_t r;

	sc = (struct clkbrd_softc *)arg;

	r = bus_space_read_1(sc->sc_bt[CLKBRD_CLK], sc->sc_bh[CLKBRD_CLK],
	    CLK_CTRL);
	if (onoff)
		r |= CLK_CTRL_RLED;
	else
		r &= ~CLK_CTRL_RLED;
	bus_space_write_1(sc->sc_bt[CLKBRD_CLK], sc->sc_bh[CLKBRD_CLK],
	    CLK_CTRL, r);
	bus_space_read_1(sc->sc_bt[CLKBRD_CLK], sc->sc_bh[CLKBRD_CLK],
	    CLK_CTRL);
}