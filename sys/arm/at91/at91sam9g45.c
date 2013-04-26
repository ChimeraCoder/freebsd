
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
#include <sys/malloc.h>
#include <sys/module.h>

#define	_ARM32_BUS_DMA_PRIVATE
#include <machine/bus.h>

#include <arm/at91/at91var.h>
#include <arm/at91/at91reg.h>
#include <arm/at91/at91soc.h>
#include <arm/at91/at91_aicreg.h>
#include <arm/at91/at91sam9g45reg.h>
#include <arm/at91/at91_pitreg.h>
#include <arm/at91/at91_pmcreg.h>
#include <arm/at91/at91_pmcvar.h>
#include <arm/at91/at91_rstreg.h>

/*
 * Standard priority levels for the system.  0 is lowest and 7 is highest.
 * These values are the ones Atmel uses for its Linux port
 */
static const int at91_irq_prio[32] =
{
	7,	/* Advanced Interrupt Controller */
	7,	/* System Peripherals */
	1,	/* Parallel IO Controller A */
	1,	/* Parallel IO Controller B */
	1,	/* Parallel IO Controller C */
	1,	/* Parallel IO Controller D and E */
	0,
	5,	/* USART 0 */
	5,	/* USART 1 */
	5,	/* USART 2 */
	5,	/* USART 3 */
	0,	/* Multimedia Card Interface 0 */
	6,	/* Two-Wire Interface 0 */
	6,	/* Two-Wire Interface 1 */
	5,	/* Serial Peripheral Interface 0 */
	5,	/* Serial Peripheral Interface 1 */
	4,	/* Serial Synchronous Controller 0 */
	4,	/* Serial Synchronous Controller 1 */
	0,	/* Timer Counter 0, 1, 2, 3, 4 and 5 */
	0,	/* Pulse Width Modulation Controller */
	0,	/* Touch Screen Controller */
	0,	/* DMA Controller */
	2,	/* USB Host High Speed port */
	3,	/* LCD Controller */
	5,	/* AC97 Controller */
	3,	/* Ethernet */
	0,	/* Image Sensor Interface */
	2,	/* USB Device High Speed port */
	0,	/* (reserved) */
	0,	/* Multimedia Card Interface 1 */
	0,	/* (reserved) */
	0,	/* Advanced Interrupt Controller IRQ0 */
};

#define DEVICE(_name, _id, _unit)		\
	{					\
		_name, _unit,			\
		AT91SAM9G45_ ## _id ##_BASE,	\
		AT91SAM9G45_ ## _id ## _SIZE,	\
		AT91SAM9G45_IRQ_ ## _id		\
	}

static const struct cpu_devs at91_devs[] =
{
	DEVICE("at91_pmc", PMC,  0),
	DEVICE("at91_wdt", WDT,  0),
	DEVICE("at91_rst", RSTC, 0),
	DEVICE("at91_pit", PIT,  0),
	DEVICE("at91_pio", PIOA, 0),
	DEVICE("at91_pio", PIOB, 1),
	DEVICE("at91_pio", PIOC, 2),
	DEVICE("at91_pio", PIOD, 3),
	DEVICE("at91_pio", PIOE, 4),
	DEVICE("at91_twi", TWI0, 0),
	DEVICE("at91_twi", TWI1, 1),
	DEVICE("at91_mci", HSMCI0, 0),
	DEVICE("at91_mci", HSMCI1, 1),
	DEVICE("uart", DBGU,   0),
	DEVICE("uart", USART0, 1),
	DEVICE("uart", USART1, 2),
	DEVICE("uart", USART2, 3),
	DEVICE("uart", USART3, 4),
	DEVICE("spi",  SPI0,   0),
	DEVICE("spi",  SPI1,   1),
	DEVICE("ate",  EMAC,   0),
	DEVICE("macb", EMAC,   0),
	DEVICE("nand", NAND,   0),
	DEVICE("ohci", OHCI,   0),
	{ 0, 0, 0, 0, 0 }
};

static void
at91_clock_init(void)
{
	struct at91_pmc_clock *clk;

	/* Update USB host port clock info */
	clk = at91_pmc_clock_ref("uhpck");
	clk->pmc_mask  = PMC_SCER_UHP_SAM9;
	at91_pmc_clock_deref(clk);

	/* Each SOC has different PLL contraints */
	clk = at91_pmc_clock_ref("plla");
	clk->pll_min_in    = SAM9G45_PLL_A_MIN_IN_FREQ;		/*   2 MHz */
	clk->pll_max_in    = SAM9G45_PLL_A_MAX_IN_FREQ;		/*  32 MHz */
	clk->pll_min_out   = SAM9G45_PLL_A_MIN_OUT_FREQ;	/* 400 MHz */
	clk->pll_max_out   = SAM9G45_PLL_A_MAX_OUT_FREQ;	/* 800 MHz */
	clk->pll_mul_shift = SAM9G45_PLL_A_MUL_SHIFT;
	clk->pll_mul_mask  = SAM9G45_PLL_A_MUL_MASK;
	clk->pll_div_shift = SAM9G45_PLL_A_DIV_SHIFT;
	clk->pll_div_mask  = SAM9G45_PLL_A_DIV_MASK;
	clk->set_outb      = at91_pmc_800mhz_plla_outb;
	at91_pmc_clock_deref(clk);
}

static struct at91_soc_data soc_data = {
	.soc_delay = at91_pit_delay,
	.soc_reset = at91_rst_cpu_reset,
	.soc_clock_init = at91_clock_init,
	.soc_irq_prio = at91_irq_prio,
	.soc_children = at91_devs,
};

AT91_SOC(AT91_T_SAM9G45, &soc_data);