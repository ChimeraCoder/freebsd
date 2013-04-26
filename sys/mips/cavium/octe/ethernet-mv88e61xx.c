
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

/*
 * Interface to the Marvell 88E61XX SMI/MDIO.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/endian.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/socket.h>

#include <dev/mii/mii.h>

#include <net/ethernet.h>
#include <net/if.h>

#include "wrapper-cvmx-includes.h"
#include "ethernet-headers.h"

#define	MV88E61XX_SMI_REG_CMD	0x00	/* Indirect command register.  */
#define	 MV88E61XX_SMI_CMD_BUSY		0x8000	/* Busy bit.  */
#define	 MV88E61XX_SMI_CMD_22		0x1000	/* Clause 22 (default 45.)  */
#define	 MV88E61XX_SMI_CMD_READ		0x0800	/* Read command.  */
#define	 MV88E61XX_SMI_CMD_WRITE	0x0400	/* Write command.  */
#define	 MV88E61XX_SMI_CMD_PHY(phy)	(((phy) & 0x1f) << 5)
#define	 MV88E61XX_SMI_CMD_REG(reg)	((reg) & 0x1f)

#define	MV88E61XX_SMI_REG_DAT	0x01	/* Indirect data register.  */

static int cvm_oct_mv88e61xx_smi_read(struct ifnet *, int, int);
static void cvm_oct_mv88e61xx_smi_write(struct ifnet *, int, int, int);
static int cvm_oct_mv88e61xx_smi_wait(struct ifnet *);

int
cvm_oct_mv88e61xx_setup_device(struct ifnet *ifp)
{
	cvm_oct_private_t *priv = (cvm_oct_private_t *)ifp->if_softc;

	priv->mdio_read = cvm_oct_mv88e61xx_smi_read;
	priv->mdio_write = cvm_oct_mv88e61xx_smi_write;
	priv->phy_device = "mv88e61xxphy";

	return (0);
}

static int
cvm_oct_mv88e61xx_smi_read(struct ifnet *ifp, int phy_id, int location)
{
	cvm_oct_private_t *priv = (cvm_oct_private_t *)ifp->if_softc;
	int error;

	error = cvm_oct_mv88e61xx_smi_wait(ifp);
	if (error != 0)
		return (0);

	cvm_oct_mdio_write(ifp, priv->phy_id, MV88E61XX_SMI_REG_CMD,
	    MV88E61XX_SMI_CMD_BUSY | MV88E61XX_SMI_CMD_22 |
	    MV88E61XX_SMI_CMD_READ | MV88E61XX_SMI_CMD_PHY(phy_id) |
	    MV88E61XX_SMI_CMD_REG(location));

	error = cvm_oct_mv88e61xx_smi_wait(ifp);
	if (error != 0)
		return (0);

	return (cvm_oct_mdio_read(ifp, priv->phy_id, MV88E61XX_SMI_REG_DAT));
}

static void
cvm_oct_mv88e61xx_smi_write(struct ifnet *ifp, int phy_id, int location, int val)
{
	cvm_oct_private_t *priv = (cvm_oct_private_t *)ifp->if_softc;

	cvm_oct_mv88e61xx_smi_wait(ifp);
	cvm_oct_mdio_write(ifp, priv->phy_id, MV88E61XX_SMI_REG_DAT, val);
	cvm_oct_mdio_write(ifp, priv->phy_id, MV88E61XX_SMI_REG_CMD,
	    MV88E61XX_SMI_CMD_BUSY | MV88E61XX_SMI_CMD_22 |
	    MV88E61XX_SMI_CMD_WRITE | MV88E61XX_SMI_CMD_PHY(phy_id) |
	    MV88E61XX_SMI_CMD_REG(location));
	cvm_oct_mv88e61xx_smi_wait(ifp);
}

static int
cvm_oct_mv88e61xx_smi_wait(struct ifnet *ifp)
{
	cvm_oct_private_t *priv = (cvm_oct_private_t *)ifp->if_softc;
	uint16_t cmd;
	unsigned i;

	for (i = 0; i < 10000; i++) {
		cmd = cvm_oct_mdio_read(ifp, priv->phy_id, MV88E61XX_SMI_REG_CMD);
		if ((cmd & MV88E61XX_SMI_CMD_BUSY) == 0)
			return (0);
	}
	return (ETIMEDOUT);
}