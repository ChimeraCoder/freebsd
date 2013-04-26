
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

/*
 * Cavium Octeon Ethernet pseudo-bus attachment.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/endian.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>

#include "ethernet-common.h"

#include "octebusvar.h"

static void		octebus_identify(driver_t *drv, device_t parent);
static int		octebus_probe(device_t dev);
static int		octebus_attach(device_t dev);
static int		octebus_detach(device_t dev);
static int		octebus_shutdown(device_t dev);

static device_method_t octebus_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify,	octebus_identify),
	DEVMETHOD(device_probe,		octebus_probe),
	DEVMETHOD(device_attach,	octebus_attach),
	DEVMETHOD(device_detach,	octebus_detach),
	DEVMETHOD(device_shutdown,	octebus_shutdown),

	/* Bus interface.  */
	DEVMETHOD(bus_add_child,	bus_generic_add_child),

	{ 0, 0 }
};

static driver_t octebus_driver = {
	"octebus",
	octebus_methods,
	sizeof (struct octebus_softc),
};

static devclass_t octebus_devclass;

DRIVER_MODULE(octebus, ciu, octebus_driver, octebus_devclass, 0, 0);

static void
octebus_identify(driver_t *drv, device_t parent)
{
	BUS_ADD_CHILD(parent, 0, "octebus", 0);
}

static int
octebus_probe(device_t dev)
{
	if (device_get_unit(dev) != 0)
		return (ENXIO);
	device_set_desc(dev, "Cavium Octeon Ethernet pseudo-bus");
	return (0);
}

static int
octebus_attach(device_t dev)
{
	struct octebus_softc *sc;
	int rv;

	sc = device_get_softc(dev);
	sc->sc_dev = dev;

	rv = cvm_oct_init_module(dev);
	if (rv != 0)
		return (ENXIO);

	return (0);
}

static int
octebus_detach(device_t dev)
{
	cvm_oct_cleanup_module(dev);
	return (0);
}

static int
octebus_shutdown(device_t dev)
{
	return (octebus_detach(dev));
}