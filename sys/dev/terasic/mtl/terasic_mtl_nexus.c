
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

#include <dev/terasic/mtl/terasic_mtl.h>

static int
terasic_mtl_nexus_probe(device_t dev)
{

	device_set_desc(dev, "Terasic Multi-touch LCD (MTL)");
	return (BUS_PROBE_DEFAULT);
}

static int
terasic_mtl_nexus_attach(device_t dev)
{
	struct terasic_mtl_softc *sc;
	u_long pixel_maddr, text_maddr, reg_maddr;
	u_long pixel_msize, text_msize, reg_msize;
	int error;

	sc = device_get_softc(dev);
	sc->mtl_dev = dev;
	sc->mtl_unit = device_get_unit(dev);

	/*
	 * Query non-standard hints to find the locations of our two memory
	 * regions.  Enforce certain alignment and size requirements.
	 */
	if (resource_long_value(device_get_name(dev), device_get_unit(dev),
	    "reg_maddr", &reg_maddr) != 0 || (reg_maddr % PAGE_SIZE != 0)) {
		device_printf(dev, "improper register address");
		return (ENXIO);
	}
	if (resource_long_value(device_get_name(dev), device_get_unit(dev),
	    "reg_msize", &reg_msize) != 0 || (reg_msize % PAGE_SIZE != 0)) {
		device_printf(dev, "improper register size");
		return (ENXIO);
	}
	if (resource_long_value(device_get_name(dev), device_get_unit(dev),
	    "pixel_maddr", &pixel_maddr) != 0 ||
	    (pixel_maddr % PAGE_SIZE != 0)) {
		device_printf(dev, "improper pixel frame buffer address");
		return (ENXIO);
	}
	if (resource_long_value(device_get_name(dev), device_get_unit(dev),
	    "pixel_msize", &pixel_msize) != 0 ||
	    (pixel_msize % PAGE_SIZE != 0)) {
		device_printf(dev, "improper pixel frame buffer size");
		return (ENXIO);
	}
	if (resource_long_value(device_get_name(dev), device_get_unit(dev),
	    "text_maddr", &text_maddr) != 0 ||
	    (text_maddr % PAGE_SIZE != 0)) {
		device_printf(dev, "improper text frame buffer address");
		return (ENXIO);
	}
	if (resource_long_value(device_get_name(dev), device_get_unit(dev),
	    "text_msize", &text_msize) != 0 ||
	    (text_msize % PAGE_SIZE != 0)) {
		device_printf(dev, "improper text frame buffer size");
		return (ENXIO);
	}

	/*
	 * Allocate resources.
	 */
	sc->mtl_reg_rid = 0;
	sc->mtl_reg_res = bus_alloc_resource(dev, SYS_RES_MEMORY,
	    &sc->mtl_reg_rid, reg_maddr, reg_maddr + reg_msize - 1,
	    reg_msize, RF_ACTIVE);
	if (sc->mtl_reg_res == NULL) {
		device_printf(dev, "couldn't map register memory\n");
		error = ENXIO;
		goto error;
	}
	device_printf(sc->mtl_dev, "registers at mem %p-%p\n",
	    (void *)reg_maddr, (void *)(reg_maddr + reg_msize));
	sc->mtl_pixel_rid = 0;
	sc->mtl_pixel_res = bus_alloc_resource(dev, SYS_RES_MEMORY,
	    &sc->mtl_pixel_rid, pixel_maddr, pixel_maddr + pixel_msize - 1,
	    pixel_msize, RF_ACTIVE);
	if (sc->mtl_pixel_res == NULL) {
		device_printf(dev, "couldn't map pixel memory\n");
		error = ENXIO;
		goto error;
	}
	device_printf(sc->mtl_dev, "pixel frame buffer at mem %p-%p\n",
	    (void *)pixel_maddr, (void *)(pixel_maddr + pixel_msize));
	sc->mtl_text_rid = 0;
	sc->mtl_text_res = bus_alloc_resource(dev, SYS_RES_MEMORY,
	    &sc->mtl_text_rid, text_maddr, text_maddr + text_msize - 1,
	    text_msize, RF_ACTIVE);
	if (sc->mtl_text_res == NULL) {
		device_printf(dev, "couldn't map text memory\n");
		error = ENXIO;
		goto error;
	}
	device_printf(sc->mtl_dev, "text frame buffer at mem %p-%p\n",
	    (void *)text_maddr, (void *)(text_maddr + text_msize));
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
terasic_mtl_nexus_detach(device_t dev)
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

static device_method_t terasic_mtl_nexus_methods[] = {
	DEVMETHOD(device_probe,		terasic_mtl_nexus_probe),
	DEVMETHOD(device_attach,	terasic_mtl_nexus_attach),
	DEVMETHOD(device_detach,	terasic_mtl_nexus_detach),
	{ 0, 0 }
};

static driver_t terasic_mtl_nexus_driver = {
	"terasic_mtl",
	terasic_mtl_nexus_methods,
	sizeof(struct terasic_mtl_softc),
};

DRIVER_MODULE(mtl, nexus, terasic_mtl_nexus_driver, terasic_mtl_devclass, 0,
    0);