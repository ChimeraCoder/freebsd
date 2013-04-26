
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

/*
 * IXP4XX Watchdog Timer Support.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/time.h>
#include <sys/bus.h>
#include <sys/resource.h>
#include <sys/rman.h>
#include <sys/watchdog.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/cpufunc.h>
#include <machine/frame.h>
#include <machine/resource.h>
#include <machine/intr.h>

#include <arm/xscale/ixp425/ixp425reg.h>
#include <arm/xscale/ixp425/ixp425var.h>

struct ixpwdog_softc {
	device_t		sc_dev;
};

static __inline uint32_t
RD4(struct ixpwdog_softc *sc, bus_size_t off)
{
	return bus_space_read_4(&ixp425_bs_tag, IXP425_TIMER_VBASE, off);
}

static __inline void
WR4(struct ixpwdog_softc *sc, bus_size_t off, uint32_t val)
{
	bus_space_write_4(&ixp425_bs_tag, IXP425_TIMER_VBASE, off, val);
}

static void
ixp425_watchdog(void *arg, u_int cmd, int *error)
{
	struct ixpwdog_softc *sc = arg;
	u_int u = cmd & WD_INTERVAL;

	WR4(sc, IXP425_OST_WDOG_KEY, OST_WDOG_KEY_MAJICK);
	if (4 <= u && u <= 35) {
		WR4(sc, IXP425_OST_WDOG_ENAB, 0);
		/* approximate 66.66MHz cycles */
		WR4(sc, IXP425_OST_WDOG, 2<<(u - 4));
		/* NB: reset on timer expiration */
		WR4(sc, IXP425_OST_WDOG_ENAB,
		    OST_WDOG_ENAB_CNT_ENA | OST_WDOG_ENAB_RST_ENA);
		*error = 0;
	} else {
		/* disable watchdog */
		WR4(sc, IXP425_OST_WDOG_ENAB, 0);
	}
	WR4(sc, IXP425_OST_WDOG_KEY, 0);
}

static int
ixpwdog_probe(device_t dev)
{
	device_set_desc(dev, "IXP4XX Watchdog Timer");
	return (0);
}

static int
ixpwdog_attach(device_t dev)
{
	struct ixpwdog_softc *sc = device_get_softc(dev);

	sc->sc_dev = dev;

	EVENTHANDLER_REGISTER(watchdog_list, ixp425_watchdog, sc, 0);
	return (0);
}

static device_method_t ixpwdog_methods[] = {
	DEVMETHOD(device_probe,		ixpwdog_probe),
	DEVMETHOD(device_attach,	ixpwdog_attach),
	{0, 0},
};

static driver_t ixpwdog_driver = {
	"ixpwdog",
	ixpwdog_methods,
	sizeof(struct ixpwdog_softc),
};
static devclass_t ixpwdog_devclass;
DRIVER_MODULE(ixpwdog, ixp, ixpwdog_driver, ixpwdog_devclass, 0, 0);