
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
 * clock (eeprom) attaches at EBus, FireHose or SBus
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/eventhandler.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/resource.h>

#include <dev/ofw/ofw_bus.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <machine/ver.h>

#include <sys/rman.h>

#include <dev/mk48txx/mk48txxvar.h>

#include "clock_if.h"

static devclass_t eeprom_devclass;

static device_probe_t eeprom_probe;
static device_attach_t eeprom_attach;

static device_method_t eeprom_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		eeprom_probe),
	DEVMETHOD(device_attach,	eeprom_attach),

	/* clock interface */
	DEVMETHOD(clock_gettime,	mk48txx_gettime),
	DEVMETHOD(clock_settime,	mk48txx_settime),

	DEVMETHOD_END
};

static driver_t eeprom_driver = {
	"eeprom",
	eeprom_methods,
	sizeof(struct mk48txx_softc),
};

DRIVER_MODULE(eeprom, ebus, eeprom_driver, eeprom_devclass, 0, 0);
DRIVER_MODULE(eeprom, fhc, eeprom_driver, eeprom_devclass, 0, 0);
DRIVER_MODULE(eeprom, sbus, eeprom_driver, eeprom_devclass, 0, 0);

static int
eeprom_probe(device_t dev)
{
	const char *name;

	name = ofw_bus_get_name(dev);
	if (strcmp(name, "eeprom") == 0 ||
	    strcmp(name, "FJSV,eeprom") == 0) {
		device_set_desc(dev, "EEPROM/clock");
		return (0);
	}
	return (ENXIO);
}

static int
eeprom_attach(device_t dev)
{
	struct mk48txx_softc *sc;
	struct timespec ts;
	int error, rid;

	sc = device_get_softc(dev);

	mtx_init(&sc->sc_mtx, "eeprom_mtx", NULL, MTX_DEF);

	rid = 0;
	sc->sc_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid,
	    RF_ACTIVE);
	if (sc->sc_res == NULL) {
		device_printf(dev, "cannot allocate resources\n");
		error = ENXIO;
		goto fail_mtx;
	}

	if ((sc->sc_model = ofw_bus_get_model(dev)) == NULL) {
		device_printf(dev, "cannot determine model\n");
		error = ENXIO;
		goto fail_res;
	}

	/* Our TOD clock year 0 is 1968. */
	sc->sc_year0 = 1968;
	/* Use default register read/write functions. */
	sc->sc_flag = 0;
	/*
	 * Generally, if the `eeprom' node has a `watchdog-enable' property
	 * this indicates that the watchdog part of the MK48T59 is usable,
	 * i.e. its RST pin is connected to the WDR input of the CPUs or
	 * something. The `eeprom' nodes of E250, E450 and the clock board
	 * variant in Exx00 have such properties. For E250 and E450 the
	 * watchdog just works, for Exx00 the delivery of the reset signal
	 * apparently has to be additionally enabled elsewhere...
	 * The OFW environment variable `watchdog-reboot?' is ignored for
	 * these watchdogs as they always trigger a system reset when they
	 * time out and can't be made to issue a break to the boot monitor
	 * instead.
	 */
	if (OF_getproplen(ofw_bus_get_node(dev), "watchdog-enable") != -1 &&
	    (strcmp(sparc64_model, "SUNW,Ultra-250") == 0 ||
	    strcmp(sparc64_model, "SUNW,Ultra-4") == 0))
		sc->sc_flag |= MK48TXX_WDOG_REGISTER | MK48TXX_WDOG_ENABLE_WDS;
	if ((error = mk48txx_attach(dev)) != 0) {
		device_printf(dev, "cannot attach time of day clock\n");
		goto fail_res;
	}

	if (bootverbose) {
		if (mk48txx_gettime(dev, &ts) != 0)
			device_printf(dev, "invalid time");
		else
			device_printf(dev, "current time: %ld.%09ld\n",
			    (long)ts.tv_sec, ts.tv_nsec);
	}

	return (0);

 fail_res:
	bus_release_resource(dev, SYS_RES_MEMORY, rid, sc->sc_res);
 fail_mtx:
	mtx_destroy(&sc->sc_mtx);

	return (error);
}