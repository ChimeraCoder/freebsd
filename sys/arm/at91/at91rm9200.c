
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
#include <arm/at91/at91rm92reg.h>
#include <arm/at91/at91_aicreg.h>
#include <arm/at91/at91_pmcreg.h>
#include <arm/at91/at91_streg.h>
#include <arm/at91/at91_pmcvar.h>
#include <arm/at91/at91soc.h>

/*
 * Standard priority levels for the system.  0 is lowest and 7 is highest.
 * These values are the ones Atmel uses for its Linux port, which differ
 * a little form the ones that are in the standard distribution.  Also,
 * the ones marked with 'TWEEK' are different based on experience.
 */
static const int at91_irq_prio[32] =
{
	7,	/* Advanced Interrupt Controller (FIQ) */
	7,	/* System Peripherals */
	1,	/* Parallel IO Controller A */
	1,	/* Parallel IO Controller B */
	1,	/* Parallel IO Controller C */
	1,	/* Parallel IO Controller D */
	5,	/* USART 0 */
	5,	/* USART 1 */
	5,	/* USART 2 */
	5,	/* USART 3 */
	0,	/* Multimedia Card Interface */
	2,	/* USB Device Port */
	4,	/* Two-Wire Interface */		/* TWEEK */
	5,	/* Serial Peripheral Interface */
	4,	/* Serial Synchronous Controller 0 */
	6,	/* Serial Synchronous Controller 1 */	/* TWEEK */
	4,	/* Serial Synchronous Controller 2 */
	0,	/* Timer Counter 0 */
	6,	/* Timer Counter 1 */			/* TWEEK */
	0,	/* Timer Counter 2 */
	0,	/* Timer Counter 3 */
	0,	/* Timer Counter 4 */
	0,	/* Timer Counter 5 */
	2,	/* USB Host port */
	3,	/* Ethernet MAC */
	0,	/* Advanced Interrupt Controller (IRQ0) */
	0,	/* Advanced Interrupt Controller (IRQ1) */
	0,	/* Advanced Interrupt Controller (IRQ2) */
	0,	/* Advanced Interrupt Controller (IRQ3) */
	0,	/* Advanced Interrupt Controller (IRQ4) */
	0,	/* Advanced Interrupt Controller (IRQ5) */
 	0	/* Advanced Interrupt Controller (IRQ6) */
};

#define DEVICE(_name, _id, _unit)		\
	{					\
		_name, _unit,			\
		AT91RM92_ ## _id ##_BASE,	\
		AT91RM92_ ## _id ## _SIZE,	\
		AT91RM92_IRQ_ ## _id		\
	}

static const struct cpu_devs at91_devs[] =
{
	DEVICE("at91_pmc",   PMC,    0),
	DEVICE("at91_st",    ST,     0),
	DEVICE("at91_pio",   PIOA,   0),
	DEVICE("at91_pio",   PIOB,   1),
	DEVICE("at91_pio",   PIOC,   2),
	DEVICE("at91_pio",   PIOD,   3),
	DEVICE("at91_rtc",   RTC,    0),

	DEVICE("at91_mci",   MCI,    0),
	DEVICE("at91_twi",   TWI,    0),
	DEVICE("at91_udp",   UDP,    0),
	DEVICE("ate",        EMAC,   0),
	DEVICE("at91_ssc",   SSC0,   0),
	DEVICE("at91_ssc",   SSC1,   1),
	DEVICE("at91_ssc",   SSC2,   2),
	DEVICE("spi",        SPI,    0),

	DEVICE("uart",       DBGU,   0),
	DEVICE("uart",       USART0, 1),
	DEVICE("uart",       USART1, 2),
	DEVICE("uart",       USART2, 3),
	DEVICE("uart",       USART3, 4),
	DEVICE("at91_aic",   AIC,    0),
	DEVICE("at91_mc",    MC,     0),
	DEVICE("at91_tc",    TC0,    0),
	DEVICE("at91_tc",    TC1,    1),
	DEVICE("ohci",       OHCI,   0),
	DEVICE("at91_cfata", CF,     0),
	{	0, 0, 0, 0, 0 }
};

static uint32_t
at91_pll_outb(int freq)
{

	if (freq > 155000000)
		return (0x0000);
	else
		return (0x8000);
}

#if 0
/* -- XXX are these needed? */
	/* Disable all interrupts for RTC (0xe24 == RTC_IDR) */
	bus_space_write_4(sc->sc_st, sc->sc_sys_sh, 0xe24, 0xffffffff);

	/* Disable all interrupts for the SDRAM controller */
	bus_space_write_4(sc->sc_st, sc->sc_sys_sh, 0xfa8, 0xffffffff);
#endif

static void
at91_clock_init(void)
{
	struct at91_pmc_clock *clk;

	/* Update USB device port clock info */
	clk = at91_pmc_clock_ref("udpck");
	clk->pmc_mask  = PMC_SCER_UDP;
	at91_pmc_clock_deref(clk);

	/* Update USB host port clock info */
	clk = at91_pmc_clock_ref("uhpck");
	clk->pmc_mask  = PMC_SCER_UHP;
	at91_pmc_clock_deref(clk);

	/* Each SOC has different PLL contraints */
	clk = at91_pmc_clock_ref("plla");
	clk->pll_min_in    = RM9200_PLL_A_MIN_IN_FREQ;		/*   1 MHz */
	clk->pll_max_in    = RM9200_PLL_A_MAX_IN_FREQ;		/*  32 MHz */
	clk->pll_min_out   = RM9200_PLL_A_MIN_OUT_FREQ;		/*  80 MHz */
	clk->pll_max_out   = RM9200_PLL_A_MAX_OUT_FREQ;		/* 180 MHz */
	clk->pll_mul_shift = RM9200_PLL_A_MUL_SHIFT;
	clk->pll_mul_mask  = RM9200_PLL_A_MUL_MASK;
	clk->pll_div_shift = RM9200_PLL_A_DIV_SHIFT;
	clk->pll_div_mask  = RM9200_PLL_A_DIV_MASK;
	clk->set_outb      = at91_pll_outb;
	at91_pmc_clock_deref(clk);

	clk = at91_pmc_clock_ref("pllb");
	clk->pll_min_in    = RM9200_PLL_B_MIN_IN_FREQ;		/* 100 KHz */
	clk->pll_max_in    = RM9200_PLL_B_MAX_IN_FREQ;		/*  32 MHz */
	clk->pll_min_out   = RM9200_PLL_B_MIN_OUT_FREQ;		/*  30 MHz */
	clk->pll_max_out   = RM9200_PLL_B_MAX_OUT_FREQ;		/* 240 MHz */
	clk->pll_mul_shift = RM9200_PLL_B_MUL_SHIFT;
	clk->pll_mul_mask  = RM9200_PLL_B_MUL_MASK;
	clk->pll_div_shift = RM9200_PLL_B_DIV_SHIFT;
	clk->pll_div_mask  = RM9200_PLL_B_DIV_MASK;
	clk->set_outb      = at91_pll_outb;
	at91_pmc_clock_deref(clk);
}

static struct at91_soc_data soc_data = {
	.soc_delay = at91_st_delay,
	.soc_reset = at91_st_cpu_reset,
	.soc_clock_init = at91_clock_init,
	.soc_irq_prio = at91_irq_prio,
	.soc_children = at91_devs,
};

AT91_SOC(AT91_T_RM9200, &soc_data);