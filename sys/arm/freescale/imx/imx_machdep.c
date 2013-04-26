
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

#include "opt_platform.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#define _ARM32_BUS_DMA_PRIVATE
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/reboot.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/bus.h>
#include <machine/frame.h> /* For trapframe_t, used in <machine/machdep.h> */
#include <machine/machdep.h>
#include <machine/pmap.h>

#include <dev/fdt/fdt_common.h>

#define	IMX51_DEV_VIRT_BASE	0xe0000000
vm_offset_t
initarm_lastaddr(void)
{

	boothowto |= RB_VERBOSE|RB_MULTIPLE;
	bootverbose = 1;

	if (fdt_immr_addr(IMX51_DEV_VIRT_BASE) != 0)
		while (1);

	/* Platform-specific initialisation */
	return (fdt_immr_va - ARM_NOCACHE_KVA_SIZE);
}

/*
 * Set initial values of GPIO output ports
 */
void
initarm_gpio_init(void)
{

}

void
initarm_late_init(void)
{

}

#define FDT_DEVMAP_MAX	2
static struct pmap_devmap fdt_devmap[FDT_DEVMAP_MAX] = {
	{ 0, 0, 0, 0, 0, },
	{ 0, 0, 0, 0, 0, }
};

/*
 * Construct pmap_devmap[] with DT-derived config data.
 */
int
platform_devmap_init(void)
{

	/*
	 * Map segment where UART1 and UART2 located.
	 */
	fdt_devmap[0].pd_va = IMX51_DEV_VIRT_BASE + 0x03f00000;
	fdt_devmap[0].pd_pa = 0x73f00000;
	fdt_devmap[0].pd_size = 0x00100000;
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

	printf("Reset ...\n");
	/* Clear n_reset flag */
	*((volatile u_int16_t *)(IMX51_DEV_VIRT_BASE + 0x03f98000)) =
	    (u_int16_t)0;
	while (1);
}