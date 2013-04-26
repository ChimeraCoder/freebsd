
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
#include <sys/types.h>
#include <sys/systm.h>

#include <mips/nlm/hal/mips-extns.h>
#include <mips/nlm/hal/haldefs.h>
#include <mips/nlm/hal/iomap.h>
#include <mips/nlm/hal/sys.h>
#include <mips/nlm/xlp.h>

uint32_t
xlp_get_cpu_frequency(int node, int core)
{
	uint64_t sysbase = nlm_get_sys_regbase(node);
	unsigned int pll_divf, pll_divr, dfs_div, ext_div;
	unsigned int rstval, dfsval;

	rstval = nlm_read_sys_reg(sysbase, SYS_POWER_ON_RESET_CFG);
	dfsval = nlm_read_sys_reg(sysbase, SYS_CORE_DFS_DIV_VALUE);
	pll_divf = ((rstval >> 10) & 0x7f) + 1;
	pll_divr = ((rstval >> 8)  & 0x3) + 1;
	if (!nlm_is_xlp8xx_ax())
		ext_div = ((rstval >> 30) & 0x3) + 1;
	else
		ext_div = 1;
	dfs_div  = ((dfsval >> (core << 2)) & 0xf) + 1;

	return ((800000000ULL * pll_divf)/(3 * pll_divr * ext_div * dfs_div));
}

static u_int
nlm_get_device_frequency(uint64_t sysbase, int devtype)
{
	uint32_t pllctrl, dfsdiv, spf, spr, div_val;
	int extra_div;

	pllctrl = nlm_read_sys_reg(sysbase, SYS_PLL_CTRL);
	if (devtype <= 7)
		div_val = nlm_read_sys_reg(sysbase, SYS_DFS_DIV_VALUE0);
	else {
		devtype -= 8;
		div_val = nlm_read_sys_reg(sysbase, SYS_DFS_DIV_VALUE1);
	}
	dfsdiv = ((div_val >> (devtype << 2)) & 0xf) + 1;
	spf = (pllctrl >> 3 & 0x7f) + 1;
	spr = (pllctrl >> 1 & 0x03) + 1;
	extra_div = nlm_is_xlp8xx_ax() ? 1 : 2;

	return ((400 * spf) / (3 * extra_div * spr * dfsdiv));
}

int
nlm_set_device_frequency(int node, int devtype, int frequency)
{
	uint64_t sysbase;
	u_int cur_freq;
	int dec_div;

	sysbase = nlm_get_sys_regbase(node);
	cur_freq = nlm_get_device_frequency(sysbase, devtype);
	if (cur_freq < (frequency - 5))
		dec_div = 1;
	else
		dec_div = 0;

	for(;;) {
		if ((cur_freq >= (frequency - 5)) && (cur_freq <= frequency))
			break;
		if (dec_div)
			nlm_write_sys_reg(sysbase, SYS_DFS_DIV_DEC_CTRL,
			    (1 << devtype));
		else
			nlm_write_sys_reg(sysbase, SYS_DFS_DIV_INC_CTRL,
			    (1 << devtype));
		cur_freq = nlm_get_device_frequency(sysbase, devtype);
	}
	return (nlm_get_device_frequency(sysbase, devtype));
}