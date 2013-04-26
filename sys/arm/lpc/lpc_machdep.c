
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

#include "opt_ddb.h"
#include "opt_platform.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#define _ARM32_BUS_DMA_PRIVATE
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/bus.h>
#include <machine/frame.h> /* For trapframe_t, used in <machine/machdep.h> */
#include <machine/machdep.h>
#include <machine/pmap.h>

#include <arm/lpc/lpcreg.h>
#include <arm/lpc/lpcvar.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ic/ns16550.h>

vm_offset_t
initarm_lastaddr(void)
{

	if (fdt_immr_addr(LPC_DEV_BASE) != 0)
		while (1);

	/* Platform-specific initialisation */
	return (fdt_immr_va - ARM_NOCACHE_KVA_SIZE);
}

void
initarm_gpio_init(void)
{

	/*
	 * Set initial values of GPIO output ports
	 */
	platform_gpio_init();
}

void
initarm_late_init(void)
{
}

#define FDT_DEVMAP_MAX	(1 + 2 + 1 + 1)
static struct pmap_devmap fdt_devmap[FDT_DEVMAP_MAX] = {
	{ 0, 0, 0, 0, 0, }
};

/*
 * Construct pmap_devmap[] with DT-derived config data.
 */
int
platform_devmap_init(void)
{

	/*
	 * IMMR range.
	 */
	fdt_devmap[0].pd_va = fdt_immr_va;
	fdt_devmap[0].pd_pa = fdt_immr_pa;
	fdt_devmap[0].pd_size = fdt_immr_size;
	fdt_devmap[0].pd_prot = VM_PROT_READ | VM_PROT_WRITE;
	fdt_devmap[0].pd_cache = PTE_NOCACHE;
	
	pmap_devmap_bootstrap_table = &fdt_devmap[0];
	return (0);
}

struct arm32_dma_range *
bus_dma_get_range(void)
{

	return (NULL);
}

int
bus_dma_get_range_nb(void)
{

	return (0);
}

void
cpu_reset(void)
{
	/* Enable WDT */
	bus_space_write_4(fdtbus_bs_tag, 
	    LPC_CLKPWR_BASE, LPC_CLKPWR_TIMCLK_CTRL,
	    LPC_CLKPWR_TIMCLK_CTRL_WATCHDOG);

	/* Instant assert of RESETOUT_N with pulse length 1ms */
	bus_space_write_4(fdtbus_bs_tag, LPC_WDTIM_BASE, LPC_WDTIM_PULSE, 13000);
	bus_space_write_4(fdtbus_bs_tag, LPC_WDTIM_BASE, LPC_WDTIM_MCTRL, 0x70);

	for (;;);
}