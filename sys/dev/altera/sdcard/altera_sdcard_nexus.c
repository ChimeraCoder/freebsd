
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
#include <sys/taskqueue.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <geom/geom_disk.h>

#include <dev/altera/sdcard/altera_sdcard.h>

/*
 * Nexus bus attachment for the Altera SD Card IP core.  Appropriate for most
 * Altera FPGA SoC-style configurations in which the IP core will be exposed
 * to the processor via a memory-mapped Avalon bus.
 */
static int
altera_sdcard_nexus_probe(device_t dev)
{

	device_set_desc(dev, "Altera Secure Data Card IP Core");
	return (BUS_PROBE_DEFAULT);
}

static int
altera_sdcard_nexus_attach(device_t dev)
{
	struct altera_sdcard_softc *sc;

	sc = device_get_softc(dev);
	sc->as_dev = dev;
	sc->as_unit = device_get_unit(dev);
	sc->as_rid = 0;
	sc->as_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->as_rid, RF_ACTIVE);
	if (sc->as_res == NULL) {
		device_printf(dev, "couldn't map memory\n");
		return (ENXIO);
	}
	altera_sdcard_attach(sc);
	return (0);
}

static int
altera_sdcard_nexus_detach(device_t dev)
{
	struct altera_sdcard_softc *sc;

	sc = device_get_softc(dev);
	KASSERT(sc->as_res != NULL, ("%s: resources not allocated",
	    __func__));
	altera_sdcard_detach(sc);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->as_rid, sc->as_res);
	return (0);
}

static device_method_t altera_sdcard_nexus_methods[] = {
	DEVMETHOD(device_probe,		altera_sdcard_nexus_probe),
	DEVMETHOD(device_attach,	altera_sdcard_nexus_attach),
	DEVMETHOD(device_detach,	altera_sdcard_nexus_detach),
	{ 0, 0 }
};

static driver_t altera_sdcard_nexus_driver = {
	"altera_sdcardc",
	altera_sdcard_nexus_methods,
	sizeof(struct altera_sdcard_softc),
};

DRIVER_MODULE(altera_sdcard, nexus, altera_sdcard_nexus_driver,
    altera_sdcard_devclass, 0, 0);