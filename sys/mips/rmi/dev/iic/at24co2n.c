
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
 * reading eeprom for the mac address .
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/bus.h>
#include <sys/resource.h>
#include <sys/rman.h>
#include <sys/sysctl.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/cpufunc.h>
#include <machine/frame.h>
#include <machine/resource.h>

#include <dev/iicbus/iiconf.h>
#include <dev/iicbus/iicbus.h>

#include "iicbus_if.h"

#define	AT24CO_EEPROM_ETH_MACADDR	0x20	

struct at24co2n_softc {
	uint32_t	sc_addr;
	device_t	sc_dev;
	uint8_t		sc_mac_addr[6];
};

static void at24co2n_read_mac(struct at24co2n_softc *);

static int
at24co2n_probe(device_t dev)
{
	device_set_desc(dev, "AT24Co2N-10SE-2.7 EEPROM for mac address");
	return (0);
}

static int
at24co2n_mac_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct at24co2n_softc *sc = arg1;
	char buf[24];
	int len;
	uint8_t *p;

	at24co2n_read_mac(sc);
	p = sc->sc_mac_addr;
	len = snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
	    p[0], p[1], p[2], p[3], p[4], p[5]);
	return SYSCTL_OUT(req, buf, len);
}


static int
at24co2n_attach(device_t dev)
{
	struct at24co2n_softc *sc = device_get_softc(dev);
	struct sysctl_ctx_list *ctx = device_get_sysctl_ctx(dev);
	struct sysctl_oid *tree = device_get_sysctl_tree(dev);

	if(sc == NULL) {
		printf("at24co2n_attach device_get_softc failed\n");
		return (0);
	}
	sc->sc_dev = dev;
	sc->sc_addr = iicbus_get_addr(dev);

	SYSCTL_ADD_PROC(ctx, SYSCTL_CHILDREN(tree), OID_AUTO,
		"eeprom-mac", CTLTYPE_STRING | CTLFLAG_RD, sc, 0,
		at24co2n_mac_sysctl, "A", "mac address");

	return (0);
}

static void
at24co2n_read_mac(struct at24co2n_softc *sc)
{
	uint8_t addr = AT24CO_EEPROM_ETH_MACADDR;
	struct iic_msg msgs[2] = {
	     { sc->sc_addr, IIC_M_WR, 1, &addr },
	     { sc->sc_addr, IIC_M_RD, 6, sc->sc_mac_addr},
	};

	iicbus_transfer(sc->sc_dev, msgs, 2);
}

static device_method_t at24co2n_methods[] = {
	DEVMETHOD(device_probe,		at24co2n_probe),
	DEVMETHOD(device_attach,	at24co2n_attach),

	{0, 0},
};

static driver_t at24co2n_driver = {
	"at24co2n",
	at24co2n_methods,
	sizeof(struct at24co2n_softc),
};
static devclass_t at24co2n_devclass;

DRIVER_MODULE(at24co2n, iicbus, at24co2n_driver, at24co2n_devclass, 0, 0);
MODULE_VERSION(at24co2n, 1);
MODULE_DEPEND(at24co2n, iicbus, 1, 1, 1);