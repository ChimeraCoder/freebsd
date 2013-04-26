
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
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/interrupt.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/rman.h>
#include <sys/malloc.h>

#include <machine/bus.h>

#include <mips/rt305x/rt305xreg.h>
#include <mips/rt305x/rt305x_icvar.h>


static int	rt305x_ic_probe(device_t);
static int	rt305x_ic_attach(device_t);
static int	rt305x_ic_detach(device_t);


static struct rt305x_ic_softc *rt305x_ic_softc = NULL;

static int
rt305x_ic_probe(device_t dev)
{
	device_set_desc(dev, "RT305X Interrupt Controller driver");
	return (0);
}

static int
rt305x_ic_attach(device_t dev)
{
	struct rt305x_ic_softc *sc = device_get_softc(dev);
	int error = 0;

	KASSERT((device_get_unit(dev) == 0),
	    ("rt305x_ic: Only one Interrupt Controller module supported"));

	if (rt305x_ic_softc != NULL)
		return (ENXIO);
	rt305x_ic_softc = sc;


	/* Map control/status registers. */
	sc->mem_rid = 0;
	sc->mem_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->mem_rid, RF_ACTIVE);

	if (sc->mem_res == NULL) {
		device_printf(dev, "couldn't map memory\n");
		error = ENXIO;
		rt305x_ic_detach(dev);
		return(error);
	}
	return (bus_generic_attach(dev));
}

static int
rt305x_ic_detach(device_t dev)
{
	struct rt305x_ic_softc *sc = device_get_softc(dev);

	bus_generic_detach(dev);

	if (sc->mem_res)
		bus_release_resource(dev, SYS_RES_MEMORY, sc->mem_rid,
		    sc->mem_res);
	return(0);
}


uint32_t
rt305x_ic_get(uint32_t reg)
{
	struct rt305x_ic_softc *sc = rt305x_ic_softc;

	if (!sc)
		return (0);

	return (bus_read_4(sc->mem_res, reg));
}

void
rt305x_ic_set(uint32_t reg, uint32_t val)
{
	struct rt305x_ic_softc *sc = rt305x_ic_softc;

	if (!sc)
		return;

	bus_write_4(sc->mem_res, reg, val);

	return;
}


static device_method_t rt305x_ic_methods[] = {
	DEVMETHOD(device_probe,			rt305x_ic_probe),
	DEVMETHOD(device_attach,		rt305x_ic_attach),
	DEVMETHOD(device_detach,		rt305x_ic_detach),

	{0, 0},
};

static driver_t rt305x_ic_driver = {
	"rt305x_ic",
	rt305x_ic_methods,
	sizeof(struct rt305x_ic_softc),
};
static devclass_t rt305x_ic_devclass;

DRIVER_MODULE(rt305x_ic, obio, rt305x_ic_driver, rt305x_ic_devclass, 0, 0);