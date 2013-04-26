
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
 * driver for homePNA PHYs
 * This is really just a stub that allows us to identify homePNA-based
 * transceicers and display the link status. MII-based homePNA PHYs
 * only support one media type and no autonegotiation. If we were
 * really clever, we could tweak some of the vendor-specific registers
 * to optimize the link.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <net/if.h>
#include <net/if_media.h>

#include <dev/mii/mii.h>
#include <dev/mii/miivar.h>
#include "miidevs.h"

#include "miibus_if.h"

static int pnaphy_probe(device_t);
static int pnaphy_attach(device_t);

static device_method_t pnaphy_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		pnaphy_probe),
	DEVMETHOD(device_attach,	pnaphy_attach),
	DEVMETHOD(device_detach,	mii_phy_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD_END
};

static devclass_t pnaphy_devclass;

static driver_t pnaphy_driver = {
	"pnaphy",
	pnaphy_methods,
	sizeof(struct mii_softc)
};

DRIVER_MODULE(pnaphy, miibus, pnaphy_driver, pnaphy_devclass, 0, 0);

static int	pnaphy_service(struct mii_softc *, struct mii_data *,int);

static const struct mii_phydesc pnaphys[] = {
	MII_PHY_DESC(yyAMD, 79c901home),
	MII_PHY_END
};

static const struct mii_phy_funcs pnaphy_funcs = {
	pnaphy_service,
	ukphy_status,
	mii_phy_reset
};

static int
pnaphy_probe(device_t dev)
{

	return (mii_phy_dev_probe(dev, pnaphys, BUS_PROBE_DEFAULT));
}

static int
pnaphy_attach(device_t dev)
{

	mii_phy_dev_attach(dev, MIIF_NOISOLATE | MIIF_IS_HPNA |
	   MIIF_NOMANPAUSE, &pnaphy_funcs, 1);
	return (0);
}

static int
pnaphy_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
{
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;

	switch (cmd) {
	case MII_POLLSTAT:
		break;

	case MII_MEDIACHG:
		/*
		 * If the interface is not up, don't do anything.
		 */
		if ((mii->mii_ifp->if_flags & IFF_UP) == 0)
			break;

		switch (IFM_SUBTYPE(ife->ifm_media)) {
		case IFM_HPNA_1:
			mii_phy_setmedia(sc);
			break;
		default:
			return (EINVAL);
		}
		break;

	case MII_TICK:
		if (mii_phy_tick(sc) == EJUSTRETURN)
			return (0);
		break;
	}

	/* Update the media status. */
	PHY_STATUS(sc);
	if (IFM_SUBTYPE(mii->mii_media_active) == IFM_10_T)
		mii->mii_media_active = IFM_ETHER | IFM_HPNA_1;

	/* Callback if something changed. */
	mii_phy_update(sc, cmd);
	return (0);
}