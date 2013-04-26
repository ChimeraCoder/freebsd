
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
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/module.h>
#include <sys/tty.h>
#include <machine/bus.h>
#include <sys/timepps.h>

#include <dev/puc/puc_bus.h>

#include <dev/sio/siovar.h>
#include <dev/sio/sioreg.h>

static	int	sio_puc_attach(device_t dev);
static	int	sio_puc_probe(device_t dev);

static device_method_t sio_puc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		sio_puc_probe),
	DEVMETHOD(device_attach,	sio_puc_attach),
	DEVMETHOD(device_detach,	siodetach),

	{ 0, 0 }
};

static driver_t sio_puc_driver = {
	sio_driver_name,
	sio_puc_methods,
	0,
};

static int
sio_puc_attach(device_t dev)
{
	uintptr_t rclk;

	if (BUS_READ_IVAR(device_get_parent(dev), dev, PUC_IVAR_CLOCK,
	    &rclk) != 0)
		rclk = DEFAULT_RCLK;
	return (sioattach(dev, 0, rclk));
}

static int
sio_puc_probe(device_t dev)
{
	device_t parent;
	uintptr_t rclk, type;
	int error;

	parent = device_get_parent(dev);

	if (BUS_READ_IVAR(parent, dev, PUC_IVAR_TYPE, &type))
		return (ENXIO);
	if (type != PUC_TYPE_SERIAL)
		return (ENXIO);

	if (BUS_READ_IVAR(parent, dev, PUC_IVAR_CLOCK, &rclk))
		rclk = DEFAULT_RCLK;
#ifdef PC98
	SET_FLAG(dev, SET_IFTYPE(COM_IF_NS16550));
#endif
	error = sioprobe(dev, 0, rclk, 1);
	return ((error > 0) ? error : BUS_PROBE_LOW_PRIORITY);
}

DRIVER_MODULE(sio, puc, sio_puc_driver, sio_devclass, 0, 0);