
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
 * Driver for the National Semiconductor DP83861, DP83865 and DP83891
 * 10/100/1000 PHYs.
 * Datasheet available at: http://www.national.com/ds/DP/DP83861.pdf
 * and at: http://www.national.com/ds/DP/DP83865.pdf
 *
 * The DP83891 is the older NS GigE PHY which isn't being sold
 * anymore.  The DP83861 is its replacement, which is an 'enhanced'
 * firmware driven component.  The major difference between the
 * two is that the DP83891 can't generate interrupts, while the
 * 83861 can (probably it wasn't originally designed to do this, but
 * it can now thanks to firmware updates).  The DP83861 also allows
 * access to its internal RAM via indirect register access.  The
 * DP83865 is an ultra low power version of the DP83861 and DP83891.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/socket.h>
#include <sys/bus.h>

#include <net/if.h>
#include <net/if_media.h>

#include <dev/mii/mii.h>
#include <dev/mii/miivar.h>
#include "miidevs.h"

#include <dev/mii/nsgphyreg.h>

#include "miibus_if.h"

static int nsgphy_probe(device_t);
static int nsgphy_attach(device_t);

static device_method_t nsgphy_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		nsgphy_probe),
	DEVMETHOD(device_attach,	nsgphy_attach),
	DEVMETHOD(device_detach,	mii_phy_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD_END
};

static devclass_t nsgphy_devclass;

static driver_t nsgphy_driver = {
	"nsgphy",
	nsgphy_methods,
	sizeof(struct mii_softc)
};

DRIVER_MODULE(nsgphy, miibus, nsgphy_driver, nsgphy_devclass, 0, 0);

static int	nsgphy_service(struct mii_softc *, struct mii_data *,int);
static void	nsgphy_status(struct mii_softc *);

static const struct mii_phydesc nsgphys[] = {
	MII_PHY_DESC(xxNATSEMI, DP83861),
	MII_PHY_DESC(xxNATSEMI, DP83865),
	MII_PHY_DESC(xxNATSEMI, DP83891),
	MII_PHY_END
};

static const struct mii_phy_funcs nsgphy_funcs = {
	nsgphy_service,
	nsgphy_status,
	mii_phy_reset
};

static int
nsgphy_probe(device_t dev)
{

	return (mii_phy_dev_probe(dev, nsgphys, BUS_PROBE_DEFAULT));
}

static int
nsgphy_attach(device_t dev)
{
	struct mii_softc *sc;

	sc = device_get_softc(dev);

	mii_phy_dev_attach(dev, MIIF_NOMANPAUSE, &nsgphy_funcs, 0);

	PHY_RESET(sc);

	/*
	 * NB: the PHY has the 10BASE-T BMSR bits hard-wired to 0,
	 * even though it supports 10BASE-T.
	 */
	sc->mii_capabilities = (PHY_READ(sc, MII_BMSR) |
	    BMSR_10TFDX | BMSR_10THDX) & sc->mii_capmask;
	/*
	 * Note that as documented manual 1000BASE-T modes of DP83865 only
	 * work together with other National Semiconductor PHYs.
	 */
	if (sc->mii_capabilities & BMSR_EXTSTAT)
		sc->mii_extcapabilities = PHY_READ(sc, MII_EXTSR);

	mii_phy_add_media(sc);
	printf("\n");

	MIIBUS_MEDIAINIT(sc->mii_dev);
	return (0);
}

static int
nsgphy_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
{

	switch (cmd) {
	case MII_POLLSTAT:
		break;

	case MII_MEDIACHG:
		/*
		 * If the interface is not up, don't do anything.
		 */
		if ((mii->mii_ifp->if_flags & IFF_UP) == 0)
			break;

		mii_phy_setmedia(sc);
		break;

	case MII_TICK:
		if (mii_phy_tick(sc) == EJUSTRETURN)
			return (0);
		break;
	}

	/* Update the media status. */
	PHY_STATUS(sc);

	/* Callback if something changed. */
	mii_phy_update(sc, cmd);
	return (0);
}

static void
nsgphy_status(struct mii_softc *sc)
{
	struct mii_data *mii = sc->mii_pdata;
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int bmsr, bmcr, physup, gtsr;

	mii->mii_media_status = IFM_AVALID;
	mii->mii_media_active = IFM_ETHER;

	bmsr = PHY_READ(sc, MII_BMSR) | PHY_READ(sc, MII_BMSR);

	physup = PHY_READ(sc, NSGPHY_MII_PHYSUP);

	if (physup & PHY_SUP_LINK)
		mii->mii_media_status |= IFM_ACTIVE;

	bmcr = PHY_READ(sc, MII_BMCR);
	if (bmcr & BMCR_ISO) {
		mii->mii_media_active |= IFM_NONE;
		mii->mii_media_status = 0;
		return;
	}

	if (bmcr & BMCR_LOOP)
		mii->mii_media_active |= IFM_LOOP;

	if (bmcr & BMCR_AUTOEN) {
		/*
		 * The media status bits are only valid if autonegotiation
		 * has completed (or it's disabled).
		 */
		if ((bmsr & BMSR_ACOMP) == 0) {
			/* Erg, still trying, I guess... */
			mii->mii_media_active |= IFM_NONE;
			return;
		}

		switch (physup & (PHY_SUP_SPEED1 | PHY_SUP_SPEED0)) {
		case PHY_SUP_SPEED1:
			mii->mii_media_active |= IFM_1000_T;
			gtsr = PHY_READ(sc, MII_100T2SR);
			if (gtsr & GTSR_MS_RES)
				mii->mii_media_active |= IFM_ETH_MASTER;
			break;

		case PHY_SUP_SPEED0:
			mii->mii_media_active |= IFM_100_TX;
			break;

		case 0:
			mii->mii_media_active |= IFM_10_T;
			break;

		default:
			mii->mii_media_active |= IFM_NONE;
			mii->mii_media_status = 0;
			return;
		}

		if (physup & PHY_SUP_DUPLEX)
			mii->mii_media_active |=
			    IFM_FDX | mii_phy_flowstatus(sc);
		else
			mii->mii_media_active |= IFM_HDX;
	} else
		mii->mii_media_active = ife->ifm_media;
}