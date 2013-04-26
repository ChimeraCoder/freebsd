
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
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/stat.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <machine/vm.h>

#include <vm/vm.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <dev/altera/avgen/altera_avgen.h>

static int
altera_avgen_fdt_probe(device_t dev)
{

	if (ofw_bus_is_compatible(dev, "sri-cambridge,avgen")) {
		device_set_desc(dev, "Generic Altera Avalon device attachment");
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

static int
altera_avgen_fdt_attach(device_t dev)
{
	struct altera_avgen_softc *sc;
	char *str_fileio, *str_mmapio;
	char *str_devname;
	phandle_t node;
	pcell_t cell;
	int devunit, error;

	sc = device_get_softc(dev);
	sc->avg_dev = dev;
	sc->avg_unit = device_get_unit(dev);

	/*
	 * Query driver-specific OpenFirmware properties to determine how to
	 * expose the device via /dev.
	 */
	str_fileio = NULL;
	str_mmapio = NULL;
	str_devname = NULL;
	devunit = -1;
	sc->avg_width = 1;
	node = ofw_bus_get_node(dev);
	if (OF_getprop(node, "sri-cambridge,width", &cell, sizeof(cell)) > 0)
		sc->avg_width = cell;
	(void)OF_getprop_alloc(node, "sri-cambridge,fileio", sizeof(char),
	    (void **)&str_fileio);
	(void)OF_getprop_alloc(node, "sri-cambridge,mmapio", sizeof(char),
	    (void **)&str_mmapio);
	(void)OF_getprop_alloc(node,  "sri-cambridge,devname", sizeof(char),
	    (void **)&str_devname);
	if (OF_getprop(node, "sri-cambridge,devunit", &cell, sizeof(cell)) > 0)
		devunit = cell;

	/* Memory allocation and checking. */
	sc->avg_rid = 0;
	sc->avg_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->avg_rid, RF_ACTIVE);
	if (sc->avg_res == NULL) {
		device_printf(dev, "couldn't map memory\n");
		return (ENXIO);
	}
	error = altera_avgen_attach(sc, str_fileio, str_mmapio, str_devname,
	    devunit);
	if (error != 0)
		bus_release_resource(dev, SYS_RES_MEMORY, sc->avg_rid,
		    sc->avg_res);
	if (str_fileio != NULL)
		free(str_fileio, M_OFWPROP);
	if (str_mmapio != NULL)
		free(str_mmapio, M_OFWPROP);
	if (str_devname != NULL)
		free(str_devname, M_OFWPROP);
	return (error);
}

static int
altera_avgen_fdt_detach(device_t dev)
{
	struct altera_avgen_softc *sc;

	sc = device_get_softc(dev);
	altera_avgen_detach(sc);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->avg_rid, sc->avg_res);
	return (0);
}

static device_method_t altera_avgen_fdt_methods[] = {
	DEVMETHOD(device_probe,		altera_avgen_fdt_probe),
	DEVMETHOD(device_attach,	altera_avgen_fdt_attach),
	DEVMETHOD(device_detach,	altera_avgen_fdt_detach),
	{ 0, 0 }
};

static driver_t altera_avgen_fdt_driver = {
	"altera_avgen",
	altera_avgen_fdt_methods,
	sizeof(struct altera_avgen_softc),
};

DRIVER_MODULE(avgen, simplebus, altera_avgen_fdt_driver,
    altera_avgen_devclass, 0, 0);