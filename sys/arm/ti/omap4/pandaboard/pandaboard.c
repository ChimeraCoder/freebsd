
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
#include <sys/kernel.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/bus.h>
#include <machine/pte.h>
#include <machine/pmap.h>
#include <machine/vmparam.h>
#include <machine/fdt.h>

#include <arm/ti/omap4/omap4var.h>
#include <arm/ti/omap4/omap4_reg.h>

/* Registers in the SCRM that control the AUX clocks */
#define SCRM_ALTCLKSRC			     (0x110)
#define SCRM_AUXCLK0                         (0x0310)
#define SCRM_AUXCLK1                         (0x0314)
#define SCRM_AUXCLK2                         (0x0318)
#define SCRM_AUXCLK3                         (0x031C)

/* Some of the GPIO register set */
#define GPIO1_OE                             (0x0134)
#define GPIO1_CLEARDATAOUT                   (0x0190)
#define GPIO1_SETDATAOUT                     (0x0194)
#define GPIO2_OE                             (0x0134)
#define GPIO2_CLEARDATAOUT                   (0x0190)
#define GPIO2_SETDATAOUT                     (0x0194)

/* Some of the PADCONF register set */
#define CONTROL_WKUP_PAD0_FREF_CLK3_OUT  (0x058)
#define CONTROL_CORE_PAD1_KPD_COL2       (0x186)
#define CONTROL_CORE_PAD0_GPMC_WAIT1     (0x08C)

#define REG_WRITE32(r, x)    *((volatile uint32_t*)(r)) = (uint32_t)(x)
#define REG_READ32(r)        *((volatile uint32_t*)(r))

#define REG_WRITE16(r, x)    *((volatile uint16_t*)(r)) = (uint16_t)(x)
#define REG_READ16(r)        *((volatile uint16_t*)(r))

/**
 *	usb_hub_init - initialises and resets the external USB hub
 *	
 *	The USB hub needs to be held in reset while the power is being applied
 *	and the reference clock is enabled at 19.2MHz.  The following is the
 *	layout of the USB hub taken from the Pandaboard reference manual.
 *
 *
 *	   .-------------.         .--------------.         .----------------.
 *	   |  OMAP4430   |         |   USB3320C   |         |    LAN9514     |
 *	   |             |         |              |         | USB Hub / Eth  |
 *	   |         CLK | <------ | CLKOUT       |         |                |
 *	   |         STP | ------> | STP          |         |                |
 *	   |         DIR | <------ | DIR          |         |                |
 *	   |         NXT | <------ | NXT          |         |                |
 *	   |        DAT0 | <-----> | DAT0         |         |                |
 *	   |        DAT1 | <-----> | DAT1      DP | <-----> | DP             |
 *	   |        DAT2 | <-----> | DAT2      DM | <-----> | DM             |
 *	   |        DAT3 | <-----> | DAT3         |         |                |
 *	   |        DAT4 | <-----> | DAT4         |         |                |
 *	   |        DAT5 | <-----> | DAT5         |  +----> | N_RESET        |
 *	   |        DAT6 | <-----> | DAT6         |  |      |                |
 *	   |        DAT7 | <-----> | DAT7         |  |      |                |
 *	   |             |         |              |  |  +-> | VDD33IO        |
 *	   |    AUX_CLK3 | ------> | REFCLK       |  |  +-> | VDD33A         |
 *	   |             |         |              |  |  |   |                |
 *	   |     GPIO_62 | --+---> | RESET        |  |  |   |                |
 *	   |             |   |     |              |  |  |   |                |
 *	   |             |   |     '--------------'  |  |   '----------------'
 *	   |             |   |     .--------------.  |  |
 *	   |             |   '---->| VOLT CONVERT |--'  |
 *	   |             |         '--------------'     |
 *	   |             |                              |
 *	   |             |         .--------------.     |
 *	   |      GPIO_1 | ------> |   TPS73633   |-----'
 *	   |             |         '--------------'
 *	   '-------------'
 *	
 *
 *	RETURNS:
 *	nothing.
 */
static void
usb_hub_init(void)
{
	bus_space_handle_t scrm_addr, gpio1_addr, gpio2_addr, scm_addr;

	if (bus_space_map(fdtbus_bs_tag, OMAP44XX_SCRM_HWBASE,
	    OMAP44XX_SCRM_SIZE, 0, &scrm_addr) != 0)
		panic("Couldn't map SCRM registers");
	if (bus_space_map(fdtbus_bs_tag, OMAP44XX_GPIO1_HWBASE, 
	    OMAP44XX_GPIO1_SIZE, 0, &gpio1_addr) != 0)
		panic("Couldn't map GPIO1 registers");
	if (bus_space_map(fdtbus_bs_tag, OMAP44XX_GPIO2_HWBASE,
	    OMAP44XX_GPIO2_SIZE, 0, &gpio2_addr) != 0)
		panic("Couldn't map GPIO2 registers");
	if (bus_space_map(fdtbus_bs_tag, OMAP44XX_SCM_PADCONF_HWBASE,
	    OMAP44XX_SCM_PADCONF_SIZE, 0, &scm_addr) != 0)
		panic("Couldn't map SCM Padconf registers");

	

	/* Need to set FREF_CLK3_OUT to 19.2 MHz and pump it out on pin GPIO_WK31.
	 * We know the SYS_CLK is 38.4Mhz and therefore to get the needed 19.2Mhz,
	 * just use a 2x divider and ensure the SYS_CLK is used as the source.
	 */
	REG_WRITE32(scrm_addr + SCRM_AUXCLK3, (1 << 16) |    /* Divider of 2 */
	                          (0 << 1) |     /* Use the SYS_CLK as the source */
	                          (1 << 8));     /* Enable the clock */

	/* Enable the clock out to the pin (GPIO_WK31). 
	 *   muxmode=fref_clk3_out, pullup/down=disabled, input buffer=disabled,
	 *   wakeup=disabled.
	 */
	REG_WRITE16(scm_addr + CONTROL_WKUP_PAD0_FREF_CLK3_OUT, 0x0000);


	/* Disable the power to the USB hub, drive GPIO1 low */
	REG_WRITE32(gpio1_addr + GPIO1_OE, REG_READ32(gpio1_addr + 
	    GPIO1_OE) & ~(1UL << 1));
	REG_WRITE32(gpio1_addr + GPIO1_CLEARDATAOUT, (1UL << 1));
	REG_WRITE16(scm_addr + CONTROL_CORE_PAD1_KPD_COL2, 0x0003);
	
	
	/* Reset the USB PHY and Hub using GPIO_62 */
	REG_WRITE32(gpio2_addr + GPIO2_OE, 
	    REG_READ32(gpio2_addr + GPIO2_OE) & ~(1UL << 30));
	REG_WRITE32(gpio2_addr + GPIO2_CLEARDATAOUT, (1UL << 30));
	REG_WRITE16(scm_addr + CONTROL_CORE_PAD0_GPMC_WAIT1, 0x0003);
	DELAY(10);
	REG_WRITE32(gpio2_addr + GPIO2_SETDATAOUT, (1UL << 30));

	
	/* Enable power to the hub (GPIO_1) */
	REG_WRITE32(gpio1_addr + GPIO1_SETDATAOUT, (1UL << 1));
	bus_space_unmap(fdtbus_bs_tag, scrm_addr, OMAP44XX_SCRM_SIZE);
	bus_space_unmap(fdtbus_bs_tag, gpio1_addr, OMAP44XX_GPIO1_SIZE);
	bus_space_unmap(fdtbus_bs_tag, gpio2_addr, OMAP44XX_GPIO2_SIZE);
	bus_space_unmap(fdtbus_bs_tag, scm_addr, OMAP44XX_SCM_PADCONF_SIZE);
}

/**
 *	board_init - initialises the pandaboard
 *	@dummy: ignored
 * 
 *	This function is called before any of the driver are initialised, which is
 *	annoying because it means we can't use the SCM, PRCM and GPIO modules which
 *	would really be useful.
 *
 *	So we don't have:
 *	   - any drivers
 *	   - no interrupts
 *
 *	What we do have:
 *	   - virt/phys mappings from the devmap (see omap4.c)
 *	   - 
 *
 *
 *	So we are hamstrung without the useful drivers and we have to go back to
 *	direct register manupulation. Luckly we don't have to do to much, basically
 *	just setup the usb hub/ethernet.
 *
 */
static void
board_init(void *dummy)
{
	/* Initialise the USB phy and hub */
	usb_hub_init();
	
	/*
	 * XXX Board identification e.g. read out from FPGA or similar should
	 * go here
	 */
}

SYSINIT(board_init, SI_SUB_CPU, SI_ORDER_THIRD, board_init, NULL);