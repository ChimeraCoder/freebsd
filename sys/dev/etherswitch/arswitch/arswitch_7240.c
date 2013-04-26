
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
#include <dev/etherswitch/arswitch/arswitch_phy.h>	/* XXX for probe */
#include <dev/etherswitch/arswitch/arswitch_7240.h>

#include "mdio_if.h"
#include "miibus_if.h"
#include "etherswitch_if.h"

/*
 * AR7240 specific functions
 */
static int
ar7240_hw_setup(struct arswitch_softc *sc)
{

	/* Enable CPU port; disable mirror port */
	arswitch_writereg(sc->sc_dev, AR8X16_REG_CPU_PORT,
	    AR8X16_CPU_PORT_EN | AR8X16_CPU_MIRROR_DIS);

	/*
	 * Let things settle; probing PHY4 doesn't seem reliable
	 * without a litle delay.
	 */
	DELAY(1000);

	return (0);
}

/*
 * Initialise other global values for the AR7240.
 */
static int
ar7240_hw_global_setup(struct arswitch_softc *sc)
{

	/* Setup TAG priority mapping */
	arswitch_writereg(sc->sc_dev, AR8X16_REG_TAG_PRIO, 0xfa50);

	/* Enable broadcast frames transmitted to the CPU */
	arswitch_writereg(sc->sc_dev, AR8X16_REG_FLOOD_MASK,
	    AR8X16_FLOOD_MASK_BCAST_TO_CPU | 0x003f003f);

	/* Setup MTU */
	arswitch_modifyreg(sc->sc_dev, AR8X16_REG_GLOBAL_CTRL,
	    AR7240_GLOBAL_CTRL_MTU_MASK,
	    SM(1536, AR7240_GLOBAL_CTRL_MTU_MASK));

	/* Service Tag */
	arswitch_modifyreg(sc->sc_dev, AR8X16_REG_SERVICE_TAG,
	    AR8X16_SERVICE_TAG_MASK, 0);

	return (0);
}

/*
 * The AR7240 probes the same as the AR8216.
 *
 * However, the support is slightly different.
 *
 * So instead of checking the PHY revision or mask register contents,
 * we simply fall back to a hint check.
 */
int
ar7240_probe(device_t dev)
{
	int is_7240 = 0;

	if (resource_int_value(device_get_name(dev), device_get_unit(dev),
	    "is_7240", &is_7240) != 0)
		return (ENXIO);

	if (is_7240 == 0)
		return (ENXIO);

	return (0);
}

void
ar7240_attach(struct arswitch_softc *sc)
{

	sc->hal.arswitch_hw_setup = ar7240_hw_setup;
	sc->hal.arswitch_hw_global_setup = ar7240_hw_global_setup;
}