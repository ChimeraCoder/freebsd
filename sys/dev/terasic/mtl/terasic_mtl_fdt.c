
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
#include <sys/consio.h>				/* struct vt_mode */
#include <sys/fbio.h>				/* video_adapter_t */
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/systm.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <dev/terasic/mtl/terasic_mtl.h>

static int
terasic_mtl_fdt_probe(device_t dev)
{

	if (ofw_bus_is_compatible(dev, "sri-cambridge,mtl")) {
		device_set_desc(dev, "Terasic Multi-touch LCD (MTL)");
		return (BUS_PROBE_DEFAULT);
	}
        return (ENXIO);
}

static int
terasic_mtl_fdt_attach(device_t dev)
{
	struct terasic_mtl_softc *sc;
	int error;

	sc = device_get_softc(dev);
	sc->mtl_dev = dev;
	sc->mtl_unit = device_get_unit(dev);

	/*
	 * FDT allows multiple memory resources to be defined for a device;
	 * query them in the order registers, pixel buffer, text buffer.
	 * However, we need to sanity-check that they are page-aligned and
	 * page-sized, so we may still abort.
	 */
	sc->mtl_reg_rid = 0;
	sc->mtl_reg_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->mtl_reg_rid, RF_ACTIVE);
	if (sc->mtl_reg_res == NULL) {
		device_printf(dev, "couldn't map register memory\n");
		error = ENXIO;
		goto error;
	}
	if (rman_get_start(sc->mtl_reg_res) % PAGE_SIZE != 0) {
		device_printf(dev, "improper register address");
		error = ENXIO;
		goto error;
	}
	if (rman_get_size(sc->mtl_reg_res) % PAGE_SIZE != 0) {
		device_printf(dev, "improper register size");
		error = ENXIO;
		goto error;
	}
	device_printf(sc->mtl_dev, "registers at mem %p-%p\n",
            (void *)rman_get_start(sc->mtl_reg_res),
	    (void *)(rman_get_start(sc->mtl_reg_res) +
	      rman_get_size(sc->mtl_reg_res)));

	sc->mtl_pixel_rid = 1;
	sc->mtl_pixel_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->mtl_pixel_rid, RF_ACTIVE);
	if (sc->mtl_pixel_res == NULL) {
		device_printf(dev, "couldn't map pixel memory\n");
		error = ENXIO;
		goto error;
	}
	if (rman_get_start(sc->mtl_pixel_res) % PAGE_SIZE != 0) {
		device_printf(dev, "improper pixel address");
		error = ENXIO;
		goto error;
	}
	if (rman_get_size(sc->mtl_pixel_res) % PAGE_SIZE != 0) {
		device_printf(dev, "improper pixel size");
		error = ENXIO;
		goto error;
	}
	device_printf(sc->mtl_dev, "pixel frame buffer at mem %p-%p\n",
            (void *)rman_get_start(sc->mtl_pixel_res),
	    (void *)(rman_get_start(sc->mtl_pixel_res) +
	      rman_get_size(sc->mtl_pixel_res)));

	sc->mtl_text_rid = 2;
	sc->mtl_text_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->mtl_text_rid, RF_ACTIVE);
	if (sc->mtl_text_res == NULL) {
		device_printf(dev, "couldn't map text memory\n");
		error = ENXIO;
		goto error;
	}
	if (rman_get_start(sc->mtl_text_res) % PAGE_SIZE != 0) {
		device_printf(dev, "improper text address");
		error = ENXIO;
		goto error;
	}
	if (rman_get_size(sc->mtl_text_res) % PAGE_SIZE != 0) {
		device_printf(dev, "improper text size");
		error = ENXIO;
		goto error;
	}
	device_printf(sc->mtl_dev, "text frame buffer at mem %p-%p\n",
            (void *)rman_get_start(sc->mtl_text_res),
	    (void *)(rman_get_start(sc->mtl_text_res) +
	      rman_get_size(sc->mtl_text_res)));

	error = terasic_mtl_attach(sc);
	if (error == 0)
		return (0);
error:
	if (sc->mtl_text_res != NULL)
		bus_release_resource(dev, SYS_RES_MEMORY, sc->mtl_text_rid,
		    sc->mtl_text_res);
	if (sc->mtl_pixel_res != NULL)
		bus_release_resource(dev, SYS_RES_MEMORY, sc->mtl_pixel_rid,
		    sc->mtl_pixel_res);
	if (sc->mtl_reg_res != NULL)
		bus_release_resource(dev, SYS_RES_MEMORY, sc->mtl_reg_rid,
		    sc->mtl_reg_res);
	return (error);
}

static int
terasic_mtl_fdt_detach(device_t dev)
{
	struct terasic_mtl_softc *sc;

	sc = device_get_softc(dev);
	terasic_mtl_detach(sc);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->mtl_text_rid,
	    sc->mtl_text_res);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->mtl_pixel_rid,
	    sc->mtl_pixel_res);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->mtl_reg_rid,
	    sc->mtl_reg_res);
	return (0);
}

static device_method_t terasic_mtl_fdt_methods[] = {
	DEVMETHOD(device_probe,		terasic_mtl_fdt_probe),
	DEVMETHOD(device_attach,	terasic_mtl_fdt_attach),
	DEVMETHOD(device_detach,	terasic_mtl_fdt_detach),
	{ 0, 0 }
};

static driver_t terasic_mtl_fdt_driver = {
	"terasic_mtl",
	terasic_mtl_fdt_methods,
	sizeof(struct terasic_mtl_softc),
};

DRIVER_MODULE(mtl, simplebus, terasic_mtl_fdt_driver, terasic_mtl_devclass, 0,
    0);