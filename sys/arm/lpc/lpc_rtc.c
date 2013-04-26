
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
#include <sys/bus.h>
#include <sys/time.h>
#include <sys/clock.h>
#include <sys/resource.h>
#include <sys/systm.h>
#include <sys/rman.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <arm/lpc/lpcreg.h>

#include "clock_if.h"

struct lpc_rtc_softc {
	device_t			lr_dev;
	struct resource	*		lr_mem_res;
	bus_space_tag_t			lr_bst;
	bus_space_handle_t		lr_bsh;
};

static int lpc_rtc_probe(device_t dev);
static int lpc_rtc_attach(device_t dev);
static int lpc_rtc_gettime(device_t dev, struct timespec *ts);
static int lpc_rtc_settime(device_t, struct timespec *);

static int
lpc_rtc_probe(device_t dev)
{

	if (!ofw_bus_is_compatible(dev, "lpc,rtc"))
		return (ENXIO);

	device_set_desc(dev, "LPC32x0 real time clock");
	return (BUS_PROBE_DEFAULT);
}

static int
lpc_rtc_attach(device_t dev)
{
	struct lpc_rtc_softc *sc = device_get_softc(dev);
	int rid = 0;

	sc->lr_dev = dev;

	clock_register(dev, 1000000);

	sc->lr_mem_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid,
	    RF_ACTIVE);
	if (!sc->lr_mem_res) {
		device_printf(dev, "cannot allocate memory window\n");
		return (ENXIO);
	}

	sc->lr_bst = rman_get_bustag(sc->lr_mem_res);
	sc->lr_bsh = rman_get_bushandle(sc->lr_mem_res);

	return (0);
}

static int
lpc_rtc_gettime(device_t dev, struct timespec *ts)
{
	struct lpc_rtc_softc *sc = device_get_softc(dev);

	ts->tv_sec = bus_space_read_4(sc->lr_bst, sc->lr_bsh, LPC_RTC_UCOUNT);
	ts->tv_nsec = 0;

	return (0);
}

static int
lpc_rtc_settime(device_t dev, struct timespec *ts)
{
	struct lpc_rtc_softc *sc = device_get_softc(dev);
	uint32_t ctrl;

	/* Stop RTC */
	ctrl = bus_space_read_4(sc->lr_bst, sc->lr_bsh, LPC_RTC_CTRL);
	bus_space_write_4(sc->lr_bst, sc->lr_bsh, LPC_RTC_CTRL, ctrl | LPC_RTC_CTRL_DISABLE);

	/* Write actual value */
	bus_space_write_4(sc->lr_bst, sc->lr_bsh, LPC_RTC_UCOUNT, ts->tv_sec);

	/* Start RTC */
	ctrl = bus_space_read_4(sc->lr_bst, sc->lr_bsh, LPC_RTC_CTRL);
	bus_space_write_4(sc->lr_bst, sc->lr_bsh, LPC_RTC_CTRL, ctrl & ~LPC_RTC_CTRL_DISABLE);

	return (0);	
}

static device_method_t lpc_rtc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		lpc_rtc_probe),
	DEVMETHOD(device_attach,	lpc_rtc_attach),

	/* Clock interface */
	DEVMETHOD(clock_gettime,	lpc_rtc_gettime),
	DEVMETHOD(clock_settime,	lpc_rtc_settime),

	{ 0, 0 },
};

static driver_t lpc_rtc_driver = {
	"rtc",
	lpc_rtc_methods,
	sizeof(struct lpc_rtc_softc),
};

static devclass_t lpc_rtc_devclass;

DRIVER_MODULE(rtc, simplebus, lpc_rtc_driver, lpc_rtc_devclass, 0, 0);