
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
#include <sys/errno.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_types.h>

#include <machine/bus.h>
#include <dev/iicbus/iic.h>
#include <dev/iicbus/iiconf.h>
#include <dev/iicbus/iicbus.h>
#include <dev/mii/mii.h>
#include <dev/mii/miivar.h>
#include <dev/etherswitch/mdio.h>

#include <dev/etherswitch/etherswitch.h>

#include <dev/etherswitch/arswitch/arswitchreg.h>
#include <dev/etherswitch/arswitch/arswitchvar.h>
#include <dev/etherswitch/arswitch/arswitch_reg.h>
#include <dev/etherswitch/arswitch/arswitch_8316.h>

#include "mdio_if.h"
#include "miibus_if.h"
#include "etherswitch_if.h"

/*
 * AR8316 specific functions
 */
static int
ar8316_hw_setup(struct arswitch_softc *sc)
{

	/*
	 * Configure the switch mode based on whether:
	 *
	 * + The switch port is GMII/RGMII;
	 * + Port 4 is either connected to the CPU or to the internal switch.
	 */
	if (sc->is_rgmii && sc->phy4cpu) {
		arswitch_writereg(sc->sc_dev, AR8X16_REG_MODE,
		    AR8X16_MODE_RGMII_PORT4_ISO);
		device_printf(sc->sc_dev,
		    "%s: MAC port == RGMII, port 4 = dedicated PHY\n",
		    __func__);
	} else if (sc->is_rgmii) {
		arswitch_writereg(sc->sc_dev, AR8X16_REG_MODE,
		    AR8X16_MODE_RGMII_PORT4_SWITCH);
		device_printf(sc->sc_dev,
		    "%s: MAC port == RGMII, port 4 = switch port\n",
		    __func__);
	} else if (sc->is_gmii) {
		arswitch_writereg(sc->sc_dev, AR8X16_REG_MODE,
		    AR8X16_MODE_GMII);
		device_printf(sc->sc_dev, "%s: MAC port == GMII\n", __func__);
	} else {
		device_printf(sc->sc_dev, "%s: unknown switch PHY config\n",
		    __func__);
		return (ENXIO);
	}

	DELAY(1000);	/* 1ms wait for things to settle */

	/*
	 * If port 4 is RGMII, force workaround
	 */
	if (sc->is_rgmii && sc->phy4cpu) {
		device_printf(sc->sc_dev,
		    "%s: port 4 RGMII workaround\n",
		    __func__);

		/* work around for phy4 rgmii mode */
		arswitch_writedbg(sc->sc_dev, 4, 0x12, 0x480c);
		/* rx delay */
		arswitch_writedbg(sc->sc_dev, 4, 0x0, 0x824e);
		/* tx delay */
		arswitch_writedbg(sc->sc_dev, 4, 0x5, 0x3d47);
		DELAY(1000);	/* 1ms, again to let things settle */
	}

	return (0);
}

/*
 * Initialise other global values, for the AR8316.
 */
static int
ar8316_hw_global_setup(struct arswitch_softc *sc)
{

	arswitch_writereg(sc->sc_dev, 0x38, 0xc000050e);

	/*
	 * Flood address table misses to all ports, and enable forwarding of
	 * broadcasts to the cpu port.
	 */
	arswitch_writereg(sc->sc_dev, AR8X16_REG_FLOOD_MASK,
	    AR8X16_FLOOD_MASK_BCAST_TO_CPU | 0x003f003f);

	arswitch_modifyreg(sc->sc_dev, AR8X16_REG_GLOBAL_CTRL,
	    AR8316_GLOBAL_CTRL_MTU_MASK, 9018 + 8 + 2);

	return (0);
}

void
ar8316_attach(struct arswitch_softc *sc)
{

	sc->hal.arswitch_hw_setup = ar8316_hw_setup;
	sc->hal.arswitch_hw_global_setup = ar8316_hw_global_setup;
}