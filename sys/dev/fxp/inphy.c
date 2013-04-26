
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
 * driver for Intel 82553 and 82555 PHYs
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

#include <dev/fxp/inphyreg.h>

#include "miibus_if.h"

static int 	inphy_probe(device_t dev);
static int 	inphy_attach(device_t dev);

static device_method_t inphy_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		inphy_probe),
	DEVMETHOD(device_attach,	inphy_attach),
	DEVMETHOD(device_detach,	mii_phy_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	{ 0, 0 }
};

static devclass_t inphy_devclass;

static driver_t inphy_driver = {
	"inphy",
	inphy_methods,
	sizeof(struct mii_softc)
};

DRIVER_MODULE(inphy, miibus, inphy_driver, inphy_devclass, 0, 0);

static int	inphy_service(struct mii_softc *, struct mii_data *, int);
static void	inphy_status(struct mii_softc *);
static void	inphy_reset(struct mii_softc *);

static const struct mii_phydesc inphys[] = {
	MII_PHY_DESC(xxINTEL, I82553),
	MII_PHY_DESC(yyINTEL, I82553),
	MII_PHY_DESC(yyINTEL, I82555),
	MII_PHY_DESC(yyINTEL, I82562EM),
	MII_PHY_DESC(yyINTEL, I82562ET),
	MII_PHY_END
};

static const struct mii_phy_funcs inphy_funcs = {
	inphy_service,
	inphy_status,
	inphy_reset
};

static int
inphy_probe(device_t dev)
{

	return (mii_phy_dev_probe(dev, inphys, BUS_PROBE_DEFAULT));
}

static int
inphy_attach(device_t dev)
{

	mii_phy_dev_attach(dev, MIIF_NOMANPAUSE, &inphy_funcs, 1);
	return (0);
}

static int
inphy_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
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
inphy_status(struct mii_softc *sc)
{
	struct mii_data *mii = sc->mii_pdata;
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int bmsr, bmcr, scr;

	mii->mii_media_status = IFM_AVALID;
	mii->mii_media_active = IFM_ETHER;

	bmsr = PHY_READ(sc, MII_BMSR) | PHY_READ(sc, MII_BMSR);
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
		if ((bmsr & BMSR_ACOMP) == 0) {
			mii->mii_media_active |= IFM_NONE;
			return;
		}

		scr = PHY_READ(sc, MII_INPHY_SCR);
		if (scr & SCR_S100)
			mii->mii_media_active |= IFM_100_TX;
		else
			mii->mii_media_active |= IFM_10_T;
		if (scr & SCR_FDX)
			mii->mii_media_active |=
			    IFM_FDX | mii_phy_flowstatus(sc);
		else
			mii->mii_media_active |= IFM_HDX;
	} else
		mii->mii_media_active = ife->ifm_media;
}

static void
inphy_reset(struct mii_softc *sc)
{

	mii_phy_reset(sc);

	/* Ensure Bay flow control is disabled. */
	PHY_WRITE(sc, MII_INPHY_SCR,
	    PHY_READ(sc, MII_INPHY_SCR) & ~SCR_FLOWCTL);
}