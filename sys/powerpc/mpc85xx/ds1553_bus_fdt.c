
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

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/clock.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/resource.h>
#include <sys/rman.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <dev/ofw/ofw_bus_subr.h>

#include "ds1553_reg.h"
#include "clock_if.h"

static devclass_t rtc_devclass;

static int rtc_attach(device_t dev);
static int rtc_probe(device_t dev);

static device_method_t rtc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		rtc_probe),
	DEVMETHOD(device_attach,	rtc_attach),

	/* clock interface */
	DEVMETHOD(clock_gettime,	ds1553_gettime),
	DEVMETHOD(clock_settime,	ds1553_settime),

	{ 0, 0 }
};

static driver_t rtc_driver = {
	"rtc",
	rtc_methods,
	sizeof(struct ds1553_softc),
};

DRIVER_MODULE(rtc, lbc, rtc_driver, rtc_devclass, 0, 0);

static int
rtc_probe(device_t dev)
{

	if (!ofw_bus_is_compatible(dev, "dallas,ds1553"))
		return (ENXIO);

	device_set_desc(dev, "Dallas Semiconductor DS1553 RTC");
	return (BUS_PROBE_DEFAULT);
}

static int
rtc_attach(device_t dev)
{
	struct timespec ts;
	struct ds1553_softc *sc;
	int error;

	sc = device_get_softc(dev);
	bzero(sc, sizeof(struct ds1553_softc));

	mtx_init(&sc->sc_mtx, "rtc_mtx", NULL, MTX_SPIN);

	sc->res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &sc->rid,
	    RF_ACTIVE);
	if (sc->res == NULL) {
		device_printf(dev, "cannot allocate resources\n");
		mtx_destroy(&sc->sc_mtx);
		return (ENXIO);
	}

	sc->sc_bst = rman_get_bustag(sc->res);
	sc->sc_bsh = rman_get_bushandle(sc->res);

	if ((error = ds1553_attach(dev)) != 0) {
		device_printf(dev, "cannot attach time of day clock\n");
		bus_release_resource(dev, SYS_RES_MEMORY, sc->rid, sc->res);
		mtx_destroy(&sc->sc_mtx);
		return (error);
	}

	clock_register(dev, 1000000);

	if (bootverbose) {
		ds1553_gettime(dev, &ts);
		device_printf(dev, "current time: %ld.%09ld\n",
		    (long)ts.tv_sec, ts.tv_nsec);
	}

	return (0);
}