
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
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/bus.h> 

#include <dev/smbus/smbconf.h>
#include <dev/smbus/smbus.h>

/*
 * Autoconfiguration and support routines for System Management bus
 */

/*
 * Device methods
 */
static int smbus_probe(device_t);
static int smbus_attach(device_t);
static int smbus_detach(device_t);

static device_method_t smbus_methods[] = {
        /* device interface */
        DEVMETHOD(device_probe,         smbus_probe),
        DEVMETHOD(device_attach,        smbus_attach),
        DEVMETHOD(device_detach,        smbus_detach),

	/* bus interface */
	DEVMETHOD(bus_add_child,	bus_generic_add_child),

	DEVMETHOD_END
};

driver_t smbus_driver = {
        "smbus",
        smbus_methods,
        sizeof(struct smbus_softc),
};

devclass_t smbus_devclass;

/*
 * At 'probe' time, we add all the devices which we know about to the
 * bus.  The generic attach routine will probe and attach them if they
 * are alive.
 */
static int
smbus_probe(device_t dev)
{

	device_set_desc(dev, "System Management Bus");

	return (0);
}

static int
smbus_attach(device_t dev)
{
	struct smbus_softc *sc = device_get_softc(dev);

	mtx_init(&sc->lock, device_get_nameunit(dev), "smbus", MTX_DEF);
	bus_generic_probe(dev);
	bus_generic_attach(dev);

	return (0);
}

static int
smbus_detach(device_t dev)
{
	struct smbus_softc *sc = device_get_softc(dev);
	int error;

	error = bus_generic_detach(dev);
	if (error)
		return (error);
	mtx_destroy(&sc->lock);

	return (0);
}

void
smbus_generic_intr(device_t dev, u_char devaddr, char low, char high, int err)
{
}

MODULE_VERSION(smbus, SMBUS_MODVER);