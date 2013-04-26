
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
 * driver for RealTek RTL8150 internal PHY
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/socket.h>
#include <sys/bus.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_media.h>

#include <dev/mii/mii.h>
#include <dev/mii/miivar.h>
#include "miidevs.h"

#include <dev/usb/net/ruephyreg.h>

#include "miibus_if.h"

static int ruephy_probe(device_t);
static int ruephy_attach(device_t);

static device_method_t ruephy_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		ruephy_probe),
	DEVMETHOD(device_attach,	ruephy_attach),
	DEVMETHOD(device_detach,	mii_phy_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD_END
};

static devclass_t ruephy_devclass;

static driver_t ruephy_driver = {
	.name = "ruephy",
	.methods = ruephy_methods,
	.size = sizeof(struct mii_softc)
};

DRIVER_MODULE(ruephy, miibus, ruephy_driver, ruephy_devclass, 0, 0);

static int ruephy_service(struct mii_softc *, struct mii_data *, int);
static void ruephy_reset(struct mii_softc *);
static void ruephy_status(struct mii_softc *);

/*
 * The RealTek RTL8150 internal PHY doesn't have vendor/device ID
 * registers; rue(4) fakes up a return value of all zeros.
 */
static const struct mii_phydesc ruephys[] = {
	{ 0, 0, "RealTek RTL8150 internal media interface" },
	MII_PHY_END
};

static const struct mii_phy_funcs ruephy_funcs = {
	ruephy_service,
	ruephy_status,
	ruephy_reset
};

static int
ruephy_probe(device_t dev)
{

	if (strcmp(device_get_name(device_get_parent(device_get_parent(dev))),
	    "rue") == 0)
		return (mii_phy_dev_probe(dev, ruephys, BUS_PROBE_DEFAULT));
	return (ENXIO);
}

static int
ruephy_attach(device_t dev)
{

	mii_phy_dev_attach(dev, MIIF_NOISOLATE | MIIF_NOMANPAUSE,
	    &ruephy_funcs, 1);
	return (0);
}

static int
ruephy_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
{
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int reg;

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
		/*
		 * Is the interface even up?
		 */
		if ((mii->mii_ifp->if_flags & IFF_UP) == 0)
			return (0);

		/*
		 * Only used for autonegotiation.
		 */
		if (IFM_SUBTYPE(ife->ifm_media) != IFM_AUTO)
			break;

		/*
		 * Check to see if we have link.  If we do, we don't
		 * need to restart the autonegotiation process.  Read
		 * the MSR twice in case it's latched.
		 */
		reg = PHY_READ(sc, RUEPHY_MII_MSR) |
		    PHY_READ(sc, RUEPHY_MII_MSR);
		if (reg & RUEPHY_MSR_LINK)
			break;

		/* Only retry autonegotiation every mii_anegticks seconds. */
		if (sc->mii_ticks <= sc->mii_anegticks)
			break;

		sc->mii_ticks = 0;
		PHY_RESET(sc);
		if (mii_phy_auto(sc) == EJUSTRETURN)
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
ruephy_reset(struct mii_softc *sc)
{

	mii_phy_reset(sc);

	/*
	 * XXX RealTek RTL8150 PHY doesn't set the BMCR properly after
	 * XXX reset, which breaks autonegotiation.
	 */
	PHY_WRITE(sc, MII_BMCR, (BMCR_S100 | BMCR_AUTOEN | BMCR_FDX));
}

static void
ruephy_status(struct mii_softc *phy)
{
	struct mii_data *mii = phy->mii_pdata;
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int bmsr, bmcr, msr;

	mii->mii_media_status = IFM_AVALID;
	mii->mii_media_active = IFM_ETHER;

	msr = PHY_READ(phy, RUEPHY_MII_MSR) | PHY_READ(phy, RUEPHY_MII_MSR);
	if (msr & RUEPHY_MSR_LINK)
		mii->mii_media_status |= IFM_ACTIVE;

	bmcr = PHY_READ(phy, MII_BMCR);
	if (bmcr & BMCR_ISO) {
		mii->mii_media_active |= IFM_NONE;
		mii->mii_media_status = 0;
		return;
	}

	bmsr = PHY_READ(phy, MII_BMSR) | PHY_READ(phy, MII_BMSR);
	if (bmcr & BMCR_AUTOEN) {
		if ((bmsr & BMSR_ACOMP) == 0) {
			/* Erg, still trying, I guess... */
			mii->mii_media_active |= IFM_NONE;
			return;
		}

		if (msr & RUEPHY_MSR_SPEED100)
			mii->mii_media_active |= IFM_100_TX;
		else
			mii->mii_media_active |= IFM_10_T;

		if (msr & RUEPHY_MSR_DUPLEX)
			mii->mii_media_active |=
			    IFM_FDX | mii_phy_flowstatus(phy);
		else
			mii->mii_media_active |= IFM_HDX;
	} else
		mii->mii_media_active = ife->ifm_media;
}