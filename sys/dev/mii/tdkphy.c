
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
 * Driver for the TDK 78Q2120 MII
 *
 * References:
 *   Datasheet for the 78Q2120 - http://www.tsc.tdk.com/lan/78q2120.pdf
 *   Most of this code stolen from ukphy.c
 */

/*
 * The TDK 78Q2120 is found on some Xircom X3201 based CardBus cards,
 * also spotted on some 3C575 cards.  It's just like any other normal
 * phy, except it does auto negotiation in a different way.
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

#include <dev/mii/tdkphyreg.h>

#include "miibus_if.h"

static int tdkphy_probe(device_t);
static int tdkphy_attach(device_t);

static device_method_t tdkphy_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		tdkphy_probe),
	DEVMETHOD(device_attach,	tdkphy_attach),
	DEVMETHOD(device_detach,	mii_phy_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD_END
};

static devclass_t tdkphy_devclass;

static driver_t tdkphy_driver = {
	"tdkphy",
	tdkphy_methods,
	sizeof(struct mii_softc)
};

DRIVER_MODULE(tdkphy, miibus, tdkphy_driver, tdkphy_devclass, 0, 0);

static int tdkphy_service(struct mii_softc *, struct mii_data *, int);
static void tdkphy_status(struct mii_softc *);

static const struct mii_phydesc tdkphys[] = {
	MII_PHY_DESC(xxTSC, 78Q2120),
	MII_PHY_END
};

static const struct mii_phy_funcs tdkphy_funcs = {
	tdkphy_service,
	tdkphy_status,
	mii_phy_reset
};

static int
tdkphy_probe(device_t dev)
{

	return (mii_phy_dev_probe(dev, tdkphys, BUS_PROBE_DEFAULT));
}

static int
tdkphy_attach(device_t dev)
{

	mii_phy_dev_attach(dev, MIIF_NOMANPAUSE, &tdkphy_funcs, 1);
	return (0);
}

static int
tdkphy_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
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
	if (sc->mii_pdata->mii_media_active & IFM_FDX)
		PHY_WRITE(sc, MII_BMCR, PHY_READ(sc, MII_BMCR) | BMCR_FDX);
	else
		PHY_WRITE(sc, MII_BMCR, PHY_READ(sc, MII_BMCR) & ~BMCR_FDX);

	/* Callback if something changed. */
	mii_phy_update(sc, cmd);
	return (0);
}

static void
tdkphy_status(struct mii_softc *phy)
{
	struct mii_data *mii = phy->mii_pdata;
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int bmsr, bmcr, anlpar, diag;

	mii->mii_media_status = IFM_AVALID;
	mii->mii_media_active = IFM_ETHER;

	bmsr = PHY_READ(phy, MII_BMSR) | PHY_READ(phy, MII_BMSR);
	if (bmsr & BMSR_LINK)
		mii->mii_media_status |= IFM_ACTIVE;

	bmcr = PHY_READ(phy, MII_BMCR);
	if (bmcr & BMCR_ISO) {
		mii->mii_media_active |= IFM_NONE;
		mii->mii_media_status = 0;
		return;
	}

	if (bmcr & BMCR_LOOP)
		mii->mii_media_active |= IFM_LOOP;

	if (bmcr & BMCR_AUTOEN) {
		/*
		 * NWay autonegotiation takes the highest-order common
		 * bit of the ANAR and ANLPAR (i.e. best media advertised
		 * both by us and our link partner).
		 */
		if ((bmsr & BMSR_ACOMP) == 0) {
			/* Erg, still trying, I guess... */
			mii->mii_media_active |= IFM_NONE;
			return;
		}

		anlpar = PHY_READ(phy, MII_ANAR) & PHY_READ(phy, MII_ANLPAR);
		/*
		 * ANLPAR doesn't get set on my card, but we check it anyway,
		 * since it is mentioned in the 78Q2120 specs.
		 */
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
		else {
			/*
			 * ANLPAR isn't set, which leaves two possibilities:
			 * 1) Auto negotiation failed
			 * 2) Auto negotiation completed, but the card forgot
			 *    to set ANLPAR.
			 * So we check the MII_DIAG(18) register...
			 */
			diag = PHY_READ(phy, MII_DIAG);
			if (diag & DIAG_NEGFAIL) /* assume 10baseT if no neg */
				mii->mii_media_active |= IFM_10_T|IFM_HDX;
			else {
				if (diag & DIAG_DUPLEX)
					mii->mii_media_active |= IFM_FDX;
				else
					mii->mii_media_active |= IFM_HDX;
				if (diag & DIAG_RATE_100)
					mii->mii_media_active |= IFM_100_TX;
				else
					mii->mii_media_active |= IFM_10_T;
			}
		}
		if ((mii->mii_media_active & IFM_FDX) != 0)
			mii->mii_media_active |= mii_phy_flowstatus(phy);
	} else
		mii->mii_media_active = ife->ifm_media;
}