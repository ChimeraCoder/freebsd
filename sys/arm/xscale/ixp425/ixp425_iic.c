
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/uio.h>

#include <arm/xscale/ixp425/ixp425reg.h>
#include <arm/xscale/ixp425/ixp425var.h>
#include <arm/xscale/ixp425/ixdp425reg.h>

#include <dev/iicbus/iiconf.h>
#include <dev/iicbus/iicbus.h>

#include "iicbb_if.h"

#define I2C_DELAY	10

/* bit clr/set shorthands */
#define	GPIO_CONF_CLR(sc, reg, mask)	\
	GPIO_CONF_WRITE_4(sc, reg, GPIO_CONF_READ_4(sc, reg) &~ (mask))
#define	GPIO_CONF_SET(sc, reg, mask)	\
	GPIO_CONF_WRITE_4(sc, reg, GPIO_CONF_READ_4(sc, reg) | (mask))

struct ixpiic_softc {
	device_t		sc_dev;
	bus_space_tag_t		sc_iot;
	bus_space_handle_t	sc_gpio_ioh;

	device_t		iicbb;
};

static struct ixpiic_softc *ixpiic_sc = NULL;

static int
ixpiic_probe(device_t dev)
{
	device_set_desc(dev, "IXP4XX GPIO-Based I2C Interface");
	return (0);
}

static int
ixpiic_attach(device_t dev)
{
	struct ixpiic_softc *sc = device_get_softc(dev);
	struct ixp425_softc *sa = device_get_softc(device_get_parent(dev));

	ixpiic_sc = sc;

	sc->sc_dev = dev;
	sc->sc_iot = sa->sc_iot;
	sc->sc_gpio_ioh = sa->sc_gpio_ioh;

	GPIO_CONF_SET(sc, IXP425_GPIO_GPOER,
		GPIO_I2C_SCL_BIT | GPIO_I2C_SDA_BIT);
	GPIO_CONF_CLR(sc, IXP425_GPIO_GPOUTR,
		GPIO_I2C_SCL_BIT | GPIO_I2C_SDA_BIT);

	/* add generic bit-banging code */	
	if ((sc->iicbb = device_add_child(dev, "iicbb", -1)) == NULL)
		device_printf(dev, "could not add iicbb\n");

	/* probe and attach the bit-banging code */
	device_probe_and_attach(sc->iicbb);

	return (0);
}

static int
ixpiic_callback(device_t dev, int index, caddr_t data)
{
	return (0);
}

static int
ixpiic_getscl(device_t dev)
{
	struct ixpiic_softc *sc = ixpiic_sc;
	uint32_t reg;

	IXP4XX_GPIO_LOCK();
	GPIO_CONF_SET(sc, IXP425_GPIO_GPOER, GPIO_I2C_SCL_BIT);

	reg = GPIO_CONF_READ_4(sc, IXP425_GPIO_GPINR);
	IXP4XX_GPIO_UNLOCK();
	return (reg & GPIO_I2C_SCL_BIT);
}

static int
ixpiic_getsda(device_t dev)
{
	struct ixpiic_softc *sc = ixpiic_sc;
	uint32_t reg;

	IXP4XX_GPIO_LOCK();
	GPIO_CONF_SET(sc, IXP425_GPIO_GPOER, GPIO_I2C_SDA_BIT);

	reg = GPIO_CONF_READ_4(sc, IXP425_GPIO_GPINR);
	IXP4XX_GPIO_UNLOCK();
	return (reg & GPIO_I2C_SDA_BIT);
}

static void
ixpiic_setsda(device_t dev, int val)
{
	struct ixpiic_softc *sc = ixpiic_sc;

	IXP4XX_GPIO_LOCK();
	GPIO_CONF_CLR(sc, IXP425_GPIO_GPOUTR, GPIO_I2C_SDA_BIT);
	if (val)
		GPIO_CONF_SET(sc, IXP425_GPIO_GPOER, GPIO_I2C_SDA_BIT);
	else
		GPIO_CONF_CLR(sc, IXP425_GPIO_GPOER, GPIO_I2C_SDA_BIT);
	IXP4XX_GPIO_UNLOCK();
	DELAY(I2C_DELAY);
}

static void
ixpiic_setscl(device_t dev, int val)
{
	struct ixpiic_softc *sc = ixpiic_sc;

	IXP4XX_GPIO_LOCK();
	GPIO_CONF_CLR(sc, IXP425_GPIO_GPOUTR, GPIO_I2C_SCL_BIT);
	if (val)
		GPIO_CONF_SET(sc, IXP425_GPIO_GPOER, GPIO_I2C_SCL_BIT);
	else
		GPIO_CONF_CLR(sc, IXP425_GPIO_GPOER, GPIO_I2C_SCL_BIT);
	IXP4XX_GPIO_UNLOCK();
	DELAY(I2C_DELAY);
}

static int
ixpiic_reset(device_t dev, u_char speed, u_char addr, u_char *oldaddr)
{
	/* reset bus */
	ixpiic_setsda(dev, 1);
	ixpiic_setscl(dev, 1);

	return (IIC_ENOADDR);
}

static device_method_t ixpiic_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		ixpiic_probe),
	DEVMETHOD(device_attach,	ixpiic_attach),

	/* iicbb interface */
	DEVMETHOD(iicbb_callback,	ixpiic_callback),
	DEVMETHOD(iicbb_setsda,		ixpiic_setsda),
	DEVMETHOD(iicbb_setscl,		ixpiic_setscl),
	DEVMETHOD(iicbb_getsda,		ixpiic_getsda),
	DEVMETHOD(iicbb_getscl,		ixpiic_getscl),
	DEVMETHOD(iicbb_reset,		ixpiic_reset),

	{ 0, 0 }
};

static driver_t ixpiic_driver = {
	"ixpiic",
	ixpiic_methods,
	sizeof(struct ixpiic_softc),
};
static devclass_t ixpiic_devclass;

DRIVER_MODULE(ixpiic, ixp, ixpiic_driver, ixpiic_devclass, 0, 0);
DRIVER_MODULE(iicbb, ixpiic, iicbb_driver, iicbb_devclass, 0, 0);