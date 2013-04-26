
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
#include <sys/condvar.h>
#include <sys/conf.h>
#include <sys/bio.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/systm.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <dev/terasic/de4led/terasic_de4led.h>

/*
 * Nexus bus attachment for the 8-element LED on the Terasic DE-4 FPGA board,
 * which is hooked up to the processor via a memory-mapped Avalon bus.
 */
static int
terasic_de4led_nexus_probe(device_t dev)
{

	device_set_desc(dev, "Terasic DE4 8-element LED");
	return (BUS_PROBE_DEFAULT);
}

static int
terasic_de4led_nexus_attach(device_t dev)
{
	struct terasic_de4led_softc *sc;

	sc = device_get_softc(dev);
	sc->tdl_dev = dev;
	sc->tdl_unit = device_get_unit(dev);
	sc->tdl_rid = 0;
	sc->tdl_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->tdl_rid, RF_ACTIVE);
	if (sc->tdl_res == NULL) {
		device_printf(dev, "couldn't map memory\n");
		return (ENXIO);
	}
	terasic_de4led_attach(sc);
	return (0);
}

static int
terasic_de4led_nexus_detach(device_t dev)
{
	struct terasic_de4led_softc *sc;

	sc = device_get_softc(dev);
	KASSERT(sc->tdl_res != NULL, ("%s: resources not allocated",
	    __func__));
	terasic_de4led_detach(sc);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->tdl_rid, sc->tdl_res);
	return (0);
}

static device_method_t terasic_de4led_nexus_methods[] = {
	DEVMETHOD(device_probe,		terasic_de4led_nexus_probe),
	DEVMETHOD(device_attach,	terasic_de4led_nexus_attach),
	DEVMETHOD(device_detach,	terasic_de4led_nexus_detach),
	{ 0, 0 }
};

static driver_t terasic_de4led_nexus_driver = {
	"terasic_de4led",
	terasic_de4led_nexus_methods,
	sizeof(struct terasic_de4led_softc),
};

DRIVER_MODULE(terasic_de4led, nexus, terasic_de4led_nexus_driver,
    terasic_de4led_devclass, 0, 0);