
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
/*
 * Cambria Front Panel LED sitting on the I2C bus.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>

#include <dev/iicbus/iiconf.h>
#include <dev/led/led.h>

#include "iicbus_if.h"

#define	IIC_M_WR	0	/* write operation */
#define	LED_ADDR	0xae	/* slave address */

struct fled_softc {
	struct cdev	*sc_led;
};

static int
fled_probe(device_t dev)
{
	device_set_desc(dev, "Gateworks Cambria Front Panel LED");
	return 0;
}

static void
fled_cb(void *arg, int onoff)
{
	uint8_t data[1];
	struct iic_msg msgs[1] = {
	     { LED_ADDR, IIC_M_WR, 1, data },
	};
	device_t dev = arg;

	data[0] = (onoff == 0);		/* NB: low true */
	(void) iicbus_transfer(dev, msgs, 1);
}

static int
fled_attach(device_t dev)
{
	struct fled_softc *sc = device_get_softc(dev);

	sc->sc_led = led_create(fled_cb, dev, "front");

	fled_cb(dev, 1);		/* Turn on LED */

	return 0;
}

static int
fled_detach(device_t dev)
{
	struct fled_softc *sc = device_get_softc(dev);

	if (sc->sc_led != NULL)
		led_destroy(sc->sc_led);

	return 0;
}

static device_method_t fled_methods[] = {
	DEVMETHOD(device_probe,		fled_probe),
	DEVMETHOD(device_attach,	fled_attach),
	DEVMETHOD(device_detach,	fled_detach),

	{0, 0},
};

static driver_t fled_driver = {
	"fled",
	fled_methods,
	sizeof(struct fled_softc),
};
static devclass_t fled_devclass;

DRIVER_MODULE(fled, iicbus, fled_driver, fled_devclass, 0, 0);
MODULE_VERSION(fled, 1);
MODULE_DEPEND(fled, iicbus, 1, 1, 1);