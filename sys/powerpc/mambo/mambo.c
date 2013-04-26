
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
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/openfirm.h>

#include <machine/bus.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <sys/rman.h>

/*
 * Mambo interface
 */
static int	mambobus_probe(device_t);
static int	mambobus_attach(device_t);

static device_method_t  mambobus_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		mambobus_probe),
	DEVMETHOD(device_attach,	mambobus_attach),

	/* Bus interface */
	DEVMETHOD(bus_add_child,	bus_generic_add_child),
	DEVMETHOD(bus_read_ivar,	bus_generic_read_ivar),
	DEVMETHOD(bus_setup_intr,	bus_generic_setup_intr),
	DEVMETHOD(bus_teardown_intr,	bus_generic_teardown_intr),
	DEVMETHOD(bus_alloc_resource,	bus_generic_alloc_resource),
	DEVMETHOD(bus_release_resource,	bus_generic_release_resource),
	DEVMETHOD(bus_activate_resource,bus_generic_activate_resource),

	DEVMETHOD_END
};

static driver_t mambobus_driver = {
	"mambo",
	mambobus_methods,
	0
};

static devclass_t mambobus_devclass;

DRIVER_MODULE(mambo, nexus, mambobus_driver, mambobus_devclass, 0, 0);

static int
mambobus_probe(device_t dev)
{
	const char *name = ofw_bus_get_name(dev);

	if (name && !strcmp(name, "mambo")) {
		device_set_desc(dev, "Mambo Simulator");
		return (0);
	}

	return (ENXIO);
}

static int
mambobus_attach(device_t dev)
{
	bus_generic_probe(dev);
	return (bus_generic_attach(dev));
}