
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
 * driver for AMD AM79c873 PHYs
 * This driver also works for Davicom DM910{1,2} PHYs, which appear
 * to be AM79c873 workalikes.
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

#include <dev/mii/amphyreg.h>

#include "miibus_if.h"

static int amphy_probe(device_t);
static int amphy_attach(device_t);

static device_method_t amphy_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		amphy_probe),
	DEVMETHOD(device_attach,	amphy_attach),
	DEVMETHOD(device_detach,	mii_phy_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD_END
};

static devclass_t amphy_devclass;

static driver_t amphy_driver = {
	"amphy",
	amphy_methods,
	sizeof(struct mii_softc)
};

DRIVER_MODULE(amphy, miibus, amphy_driver, amphy_devclass, 0, 0);

static int	amphy_service(struct mii_softc *, struct mii_data *, int);
static void	amphy_status(struct mii_softc *);

static const struct mii_phydesc amphys[] = {
	MII_PHY_DESC(xxDAVICOM, DM9102),
	MII_PHY_DESC(xxDAVICOM, DM9101),
	MII_PHY_DESC(yyDAVICOM, DM9101),
	MII_PHY_END
};

static const struct mii_phy_funcs amphy_funcs = {
	amphy_service,
	amphy_status,
	mii_phy_reset
};

static int
amphy_probe(device_t dev)
{

	return (mii_phy_dev_probe(dev, amphys, BUS_PROBE_DEFAULT));
}

static int
amphy_attach(device_t dev)
{

	mii_phy_dev_attach(dev, MIIF_NOMANPAUSE, &amphy_funcs, 1);
	return (0);
}

static int
amphy_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
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
amphy_status(struct mii_softc *sc)
{
	struct mii_data *mii = sc->mii_pdata;
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int bmsr, bmcr, par, anlpar;

	mii->mii_media_status = IFM_AVALID;
	mii->mii_media_active = IFM_ETHER;

	bmsr = PHY_READ(sc, MII_BMSR) |
	    PHY_READ(sc, MII_BMSR);
	if (bmsr & BMSR_LINK)
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
		 * The PAR status bits are only valid if autonegotiation
		 * has completed (or it's disabled).
		 */
		if ((bmsr & BMSR_ACOMP) == 0) {
			/* Erg, still trying, I guess... */
			mii->mii_media_active |= IFM_NONE;
			return;
		}

		if (PHY_READ(sc, MII_ANER) & ANER_LPAN) {
			anlpar = PHY_READ(sc, MII_ANAR) &
			    PHY_READ(sc, MII_ANLPAR);
			if (anlpar & ANLPAR_TX_FD)
				mii->mii_media_active |= IFM_100_TX|IFM_FDX;
			else if (anlpar & ANLPAR_T4)
				mii->mii_media_active |= IFM_100_T4|IFM_HDX;
			else if (anlpar & ANLPAR_TX)
				mii->mii_media_active |= IFM_100_TX|IFM_HDX;
			else if (anlpar & ANLPAR_10_FD)
				mii->mii_media_active |= IFM_10_T|IFM_FDX;
			else if (anlpar & ANLPAR_10)
				mii->mii_media_active |= IFM_10_T|IFM_HDX;
			else
				mii->mii_media_active |= IFM_NONE;
			return;
		}

		/*
		 * Link partner is not capable of autonegotiation.
		 */
		par = PHY_READ(sc, MII_AMPHY_DSCSR);
		if (par & DSCSR_100FDX)
			mii->mii_media_active |= IFM_100_TX|IFM_FDX;
		else if (par & DSCSR_100HDX)
			mii->mii_media_active |= IFM_100_TX|IFM_HDX;
		else if (par & DSCSR_10FDX)
			mii->mii_media_active |= IFM_10_T|IFM_HDX;
		else if (par & DSCSR_10HDX)
			mii->mii_media_active |= IFM_10_T|IFM_HDX;
		if ((mii->mii_media_active & IFM_FDX) != 0)
			mii->mii_media_active |= mii_phy_flowstatus(sc);
	} else
		mii->mii_media_active = ife->ifm_media;
}