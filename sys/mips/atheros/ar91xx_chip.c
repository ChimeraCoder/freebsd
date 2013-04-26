
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

#include "opt_ddb.h"

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/cons.h>
#include <sys/kdb.h>
#include <sys/reboot.h>

#include <vm/vm.h>
#include <vm/vm_page.h>

#include <net/ethernet.h>

#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/cpuregs.h>
#include <machine/hwfunc.h>
#include <machine/md_var.h>
#include <machine/trap.h>
#include <machine/vmparam.h>

#include <mips/atheros/ar71xxreg.h>
#include <mips/atheros/ar71xx_cpudef.h>
#include <mips/atheros/ar71xx_chip.h>
#include <mips/atheros/ar91xxreg.h>
#include <mips/atheros/ar91xx_chip.h>

#include <mips/sentry5/s5reg.h>

static void
ar91xx_chip_detect_mem_size(void)
{
}

static void
ar91xx_chip_detect_sys_frequency(void)
{
	uint32_t pll;
	uint32_t freq;
	uint32_t div;

	u_ar71xx_refclk = AR91XX_BASE_FREQ;

	pll = ATH_READ_REG(AR91XX_PLL_REG_CPU_CONFIG);

	div = ((pll >> AR91XX_PLL_DIV_SHIFT) & AR91XX_PLL_DIV_MASK);
	freq = div * AR91XX_BASE_FREQ;
	u_ar71xx_cpu_freq = freq;

	div = ((pll >> AR91XX_DDR_DIV_SHIFT) & AR91XX_DDR_DIV_MASK) + 1;
	u_ar71xx_ddr_freq = freq / div;

	div = (((pll >> AR91XX_AHB_DIV_SHIFT) & AR91XX_AHB_DIV_MASK) + 1) * 2;
	u_ar71xx_ahb_freq = u_ar71xx_cpu_freq / div;
}

static void
ar91xx_chip_device_stop(uint32_t mask)
{
	uint32_t reg;

	reg = ATH_READ_REG(AR91XX_RESET_REG_RESET_MODULE);
	ATH_WRITE_REG(AR91XX_RESET_REG_RESET_MODULE, reg | mask);
}

static void
ar91xx_chip_device_start(uint32_t mask)
{
	uint32_t reg;

	reg = ATH_READ_REG(AR91XX_RESET_REG_RESET_MODULE);
	ATH_WRITE_REG(AR91XX_RESET_REG_RESET_MODULE, reg & ~mask);
}

static int
ar91xx_chip_device_stopped(uint32_t mask)
{
	uint32_t reg;

	reg = ATH_READ_REG(AR91XX_RESET_REG_RESET_MODULE);
	return ((reg & mask) == mask);
}

static void
ar91xx_chip_set_pll_ge(int unit, int speed, uint32_t pll)
{

	switch (unit) {
	case 0:
		ar71xx_write_pll(AR91XX_PLL_REG_ETH_CONFIG,
		    AR91XX_PLL_REG_ETH0_INT_CLOCK, pll,
		    AR91XX_ETH0_PLL_SHIFT);
		break;
	case 1:
		ar71xx_write_pll(AR91XX_PLL_REG_ETH_CONFIG,
		    AR91XX_PLL_REG_ETH1_INT_CLOCK, pll,
		    AR91XX_ETH1_PLL_SHIFT);
		break;
	default:
		printf("%s: invalid PLL set for arge unit: %d\n",
		    __func__, unit);
		return;
	}
}

static void
ar91xx_chip_ddr_flush_ge(int unit)
{

	switch (unit) {
	case 0:
		ar71xx_ddr_flush(AR91XX_DDR_REG_FLUSH_GE0);
		break;
	case 1:
		ar71xx_ddr_flush(AR91XX_DDR_REG_FLUSH_GE1);
		break;
	default:
		printf("%s: invalid DDR flush for arge unit: %d\n",
		    __func__, unit);
		return;
	}
}

static void
ar91xx_chip_ddr_flush_ip2(void)
{

	ar71xx_ddr_flush(AR91XX_DDR_REG_FLUSH_WMAC);
}


static uint32_t
ar91xx_chip_get_eth_pll(unsigned int mac, int speed)
{
	uint32_t pll;

	switch(speed) {
	case 10:
		pll = AR91XX_PLL_VAL_10;
		break;
	case 100:
		pll = AR91XX_PLL_VAL_100;
		break;
	case 1000:
		pll = AR91XX_PLL_VAL_1000;
		break;
	default:
		printf("%s%d: invalid speed %d\n", __func__, mac, speed);
		pll = 0;
	}

	return (pll);
}

static void
ar91xx_chip_init_usb_peripheral(void)
{

	ar71xx_device_stop(AR91XX_RST_RESET_MODULE_USBSUS_OVERRIDE);
	DELAY(100);

	ar71xx_device_start(RST_RESET_USB_HOST);
	DELAY(100);

	ar71xx_device_start(RST_RESET_USB_PHY);
	DELAY(100);

	/* Wireless */
	ar71xx_device_stop(AR91XX_RST_RESET_MODULE_AMBA2WMAC);
	DELAY(1000);

	ar71xx_device_start(AR91XX_RST_RESET_MODULE_AMBA2WMAC);
	DELAY(1000);
}

struct ar71xx_cpu_def ar91xx_chip_def = {
	&ar91xx_chip_detect_mem_size,
	&ar91xx_chip_detect_sys_frequency,
	&ar91xx_chip_device_stop,
	&ar91xx_chip_device_start,
	&ar91xx_chip_device_stopped,
	&ar91xx_chip_set_pll_ge,
	&ar71xx_chip_set_mii_speed,
	&ar71xx_chip_set_mii_if,
	&ar91xx_chip_ddr_flush_ge,
	&ar91xx_chip_get_eth_pll,
	&ar91xx_chip_ddr_flush_ip2,
	&ar91xx_chip_init_usb_peripheral,
};