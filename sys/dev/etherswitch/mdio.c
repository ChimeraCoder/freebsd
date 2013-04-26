
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

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/systm.h>

#include <dev/etherswitch/mdio.h>

#include "mdio_if.h"

static void
mdio_identify(driver_t *driver, device_t parent)
{

	if (device_find_child(parent, mdio_driver.name, -1) == NULL)
		BUS_ADD_CHILD(parent, 0, mdio_driver.name, -1);
}

static int
mdio_probe(device_t dev)
{

	device_set_desc(dev, "MDIO");

	return (BUS_PROBE_SPECIFIC);
}

static int
mdio_attach(device_t dev)
{

	bus_generic_probe(dev);
	bus_enumerate_hinted_children(dev);
	return (bus_generic_attach(dev));
}

static int
mdio_detach(device_t dev)
{

	bus_generic_detach(dev);
	return (0);
}

static int
mdio_readreg(device_t dev, int phy, int reg)
{

	return (MDIO_READREG(device_get_parent(dev), phy, reg));
}

static int
mdio_writereg(device_t dev, int phy, int reg, int val)
{

	return (MDIO_WRITEREG(device_get_parent(dev), phy, reg, val));
}

static void
mdio_hinted_child(device_t dev, const char *name, int unit)
{

	device_add_child(dev, name, unit);
}

static device_method_t mdio_methods[] = {
	/* device interface */
	DEVMETHOD(device_identify,	mdio_identify),
	DEVMETHOD(device_probe,		mdio_probe),
	DEVMETHOD(device_attach,	mdio_attach),
	DEVMETHOD(device_detach,	mdio_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),

	/* bus interface */
	DEVMETHOD(bus_add_child,	device_add_child_ordered),
	DEVMETHOD(bus_hinted_child,	mdio_hinted_child),

	/* MDIO access */
	DEVMETHOD(mdio_readreg,		mdio_readreg),
	DEVMETHOD(mdio_writereg,	mdio_writereg),

	DEVMETHOD_END
};

driver_t mdio_driver = {
	"mdio",
	mdio_methods,
	0
};

devclass_t mdio_devclass;