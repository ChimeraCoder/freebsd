
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
#include <machine/resource.h>
#include <sys/timepps.h>

#include <dev/pccard/pccard_cis.h>
#include <dev/pccard/pccardvar.h>

#include <dev/sio/siovar.h>

static	int	sio_pccard_attach(device_t dev);
static	int	sio_pccard_probe(device_t dev);

static device_method_t sio_pccard_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		sio_pccard_probe),
	DEVMETHOD(device_attach,	sio_pccard_attach),
	DEVMETHOD(device_detach,	siodetach),

	{ 0, 0 }
};

static driver_t sio_pccard_driver = {
	sio_driver_name,
	sio_pccard_methods,
	0,
};

static int
sio_pccard_probe(device_t dev)
{
	int		error = 0;
	u_int32_t	fcn = PCCARD_FUNCTION_UNSPEC;

	error = pccard_get_function(dev, &fcn);
	if (error != 0)
		return (error);

	/*
	 * If a serial card, we are likely the right driver.  However,
	 * some serial cards are better servered by other drivers, so
	 * allow other drivers to claim it, if they want.
	 */
	if (fcn == PCCARD_FUNCTION_SERIAL)
		return (-100);
	return (ENXIO);
}

static int
sio_pccard_attach(device_t dev)
{
	int err;

#ifdef PC98
	SET_FLAG(dev, SET_IFTYPE(COM_IF_MODEM_CARD));
#endif
	/* Do not probe IRQ - pccard doesn't turn on the interrupt line */
	/* until bus_setup_intr */
	if ((err = sioprobe(dev, 0, 0UL, 1)) > 0)
		return (err);
	return (sioattach(dev, 0, 0UL));
}

DRIVER_MODULE(sio, pccard, sio_pccard_driver, sio_devclass, 0, 0);