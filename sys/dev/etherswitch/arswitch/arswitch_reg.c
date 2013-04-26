
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

#include "mdio_if.h"
#include "miibus_if.h"
#include "etherswitch_if.h"

static inline void
arswitch_split_setpage(device_t dev, uint32_t addr, uint16_t *phy,
    uint16_t *reg)
{
	struct arswitch_softc *sc = device_get_softc(dev);
	uint16_t page;

	page = ((addr) >> 9) & 0xffff;
	*phy = (((addr) >> 6) & 0x07) | 0x10;
	*reg = ((addr) >> 1) & 0x1f;

	/*
	 * The earlier code would only switch the page
	 * over if the page were different.  Experiments have
	 * shown that this is unstable.
	 *
	 * Hence, the page is always set here.
	 *
	 * See PR kern/172968
	 */
	MDIO_WRITEREG(device_get_parent(dev), 0x18, 0, page);
	sc->page = page;
}

/*
 * Read half a register.  Some of the registers define control bits, and
 * the sequence of half-word accesses matters.  The register addresses
 * are word-even (mod 4).
 */
static inline int
arswitch_readreg16(device_t dev, int addr)
{
	uint16_t phy, reg;
	
	arswitch_split_setpage(dev, addr, &phy, &reg);
	return (MDIO_READREG(device_get_parent(dev), phy, reg));
}

/*
 * XXX NOTE:
 *
 * This may not work for AR7240 series embedded switches -
 * the per-PHY register space doesn't seem to be exposed.
 *
 * In that instance, it may be required to speak via
 * the internal switch PHY MDIO bus indirection.
 */
void
arswitch_writedbg(device_t dev, int phy, uint16_t dbg_addr,
    uint16_t dbg_data)
{
	(void) MDIO_WRITEREG(device_get_parent(dev), phy,
	    MII_ATH_DBG_ADDR, dbg_addr);
	(void) MDIO_WRITEREG(device_get_parent(dev), phy,
	    MII_ATH_DBG_DATA, dbg_data);
}

/*
 * Write half a register
 */
static inline int
arswitch_writereg16(device_t dev, int addr, int data)
{
	uint16_t phy, reg;
	
	arswitch_split_setpage(dev, addr, &phy, &reg);
	return (MDIO_WRITEREG(device_get_parent(dev), phy, reg, data));
}

int
arswitch_readreg_lsb(device_t dev, int addr)
{

	return (arswitch_readreg16(dev, addr));
}

int
arswitch_readreg_msb(device_t dev, int addr)
{

	return (arswitch_readreg16(dev, addr + 2) << 16);
}

int
arswitch_writereg_lsb(device_t dev, int addr, int data)
{

	return (arswitch_writereg16(dev, addr, data & 0xffff));
}

int
arswitch_writereg_msb(device_t dev, int addr, int data)
{

	return (arswitch_writereg16(dev, addr + 2, (data >> 16) & 0xffff));
}

int
arswitch_readreg(device_t dev, int addr)
{

	return (arswitch_readreg_lsb(dev, addr) |
	    arswitch_readreg_msb(dev, addr));
}

int
arswitch_writereg(device_t dev, int addr, int value)
{

	/* XXX Check the first write too? */
	arswitch_writereg_lsb(dev, addr, value);
	return (arswitch_writereg_msb(dev, addr, value));
}

int
arswitch_modifyreg(device_t dev, int addr, int mask, int set)
{
	int value;
	
	value = arswitch_readreg(dev, addr);
	value &= ~mask;
	value |= set;
	return (arswitch_writereg(dev, addr, value));
}