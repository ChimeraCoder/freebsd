
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

#include <dev/isf/isf.h>

/*
 * Nexus bus attachment for the Intel Strata Flash devices.  Appropriate for
 * most Altera FPGA SoC-style configurations in which the part will be exposed
 * to the processor via a memory-mapped Avalon bus.
 */
static int
isf_nexus_probe(device_t dev)
{

	device_set_desc(dev, "Intel StrataFlash NOR flash device");
	return (BUS_PROBE_DEFAULT);
}

static int
isf_nexus_attach(device_t dev)
{
	int			 error;
	struct isf_softc	*sc;

	sc = device_get_softc(dev);
	sc->isf_dev = dev;
	sc->isf_unit = device_get_unit(dev);
	sc->isf_rid = 0;
	sc->isf_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->isf_rid, RF_ACTIVE);
	if (sc->isf_res == NULL) {
		device_printf(dev, "couldn't map memory\n");
		return (ENXIO);
	}
	error = isf_attach(sc);
	if (error)
		bus_release_resource(dev, SYS_RES_MEMORY, sc->isf_rid,
		    sc->isf_res);
	return (error);
}

static int
isf_nexus_detach(device_t dev)
{
	struct isf_softc *sc;

	sc = device_get_softc(dev);
	KASSERT(sc->isf_res != NULL, ("%s: resources not allocated",
	    __func__));
	isf_detach(sc);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->isf_rid, sc->isf_res);
	return (0);
}

static device_method_t isf_nexus_methods[] = {
	DEVMETHOD(device_probe,		isf_nexus_probe),
	DEVMETHOD(device_attach,	isf_nexus_attach),
	DEVMETHOD(device_detach,	isf_nexus_detach),
	{ 0, 0 }
};

static driver_t isf_nexus_driver = {
	"isf",
	isf_nexus_methods,
	sizeof(struct isf_softc),
};

DRIVER_MODULE(isf, nexus, isf_nexus_driver, isf_devclass, 0, 0);