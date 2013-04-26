
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

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/bus.h>
#include <machine/frame.h> /* For trapframe_t, used in <machine/machdep.h> */
#include <machine/machdep.h>
#include <machine/pmap.h>

#include <arm/ti/omap4/omap4_reg.h>

/* Start of address space used for bootstrap map */
#define DEVMAP_BOOTSTRAP_MAP_START	0xE0000000

void (*ti_cpu_reset)(void);

vm_offset_t
initarm_lastaddr(void)
{

	ti_cpu_reset = NULL;
	return (DEVMAP_BOOTSTRAP_MAP_START - ARM_NOCACHE_KVA_SIZE);
}

void
initarm_gpio_init(void)
{
}

void
initarm_late_init(void)
{
}

#define FDT_DEVMAP_MAX	(2)		// FIXME
static struct pmap_devmap fdt_devmap[FDT_DEVMAP_MAX] = {
	{ 0, 0, 0, 0, 0, }
};


/*
 * Construct pmap_devmap[] with DT-derived config data.
 */
int
platform_devmap_init(void)
{
	int i = 0;
#if defined(SOC_OMAP4)
	fdt_devmap[i].pd_va = 0xE8000000;
	fdt_devmap[i].pd_pa = 0x48000000;
	fdt_devmap[i].pd_size = 0x1000000;
	fdt_devmap[i].pd_prot = VM_PROT_READ | VM_PROT_WRITE;
	fdt_devmap[i].pd_cache = PTE_DEVICE;
	i++;
#elif defined(SOC_TI_AM335X)
	fdt_devmap[i].pd_va = 0xE4C00000;
	fdt_devmap[i].pd_pa = 0x44C00000;       /* L4_WKUP */
	fdt_devmap[i].pd_size = 0x400000;       /* 4 MB */
	fdt_devmap[i].pd_prot = VM_PROT_READ | VM_PROT_WRITE;
	fdt_devmap[i].pd_cache = PTE_DEVICE;
	i++;
#else
#error "Unknown SoC"
#endif

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
cpu_reset()
{
	if (ti_cpu_reset)
		(*ti_cpu_reset)();
	else
		printf("no cpu_reset implementation\n");
	printf("Reset failed!\n");
	while (1);
}