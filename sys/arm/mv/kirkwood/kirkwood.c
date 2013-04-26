
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>

#include <machine/bus.h>

#include <arm/mv/mvreg.h>
#include <arm/mv/mvvar.h>
#include <arm/mv/mvwin.h>

struct resource_spec mv_gpio_res[] = {
	{ SYS_RES_MEMORY,	0,	RF_ACTIVE },
	{ SYS_RES_IRQ,		0,	RF_ACTIVE },
	{ SYS_RES_IRQ,		1,	RF_ACTIVE },
	{ SYS_RES_IRQ,		2,	RF_ACTIVE },
	{ SYS_RES_IRQ,		3,	RF_ACTIVE },
	{ SYS_RES_IRQ,		4,	RF_ACTIVE },
	{ SYS_RES_IRQ,		5,	RF_ACTIVE },
	{ SYS_RES_IRQ,		6,	RF_ACTIVE },
	{ -1, 0 }
};

const struct decode_win xor_win_tbl[] = {
	{ 0 },
};
const struct decode_win *xor_wins = xor_win_tbl;
int xor_wins_no = 0;

uint32_t
get_tclk(void)
{
	uint32_t dev, rev;

	/*
	 * On Kirkwood TCLK is not configurable and depends on silicon
	 * revision:
	 * - A0 and A1 have TCLK hardcoded to 200 MHz.
	 * - Z0 and others have TCLK hardcoded to 166 MHz.
	 */
	soc_id(&dev, &rev);
	if (dev == MV_DEV_88F6281 && (rev == 2 || rev == 3))
		return (TCLK_200MHZ);
	if (dev == MV_DEV_88F6282)
		return (TCLK_200MHZ);

	return (TCLK_166MHZ);
}