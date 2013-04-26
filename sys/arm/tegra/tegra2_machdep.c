
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

#include <dev/fdt/fdt_common.h>

/* FIXME move to tegrareg.h */
#define TEGRA2_BASE			0xE0000000	/* KVM base for peripherials */
#define TEGRA2_UARTA_VA_BASE		0xE0006000
#define TEGRA2_UARTA_PA_BASE		0x70006000

#define TEGRA2_CLK_RST_PA_BASE		0x60006000

#define TEGRA2_CLK_RST_OSC_FREQ_DET_REG		0x58
#define TEGRA2_CLK_RST_OSC_FREQ_DET_STAT_REG	0x5C
#define OSC_FREQ_DET_TRIG			(1<<31)
#define OSC_FREQ_DET_BUSY               	(1<<31)

#if 0
static int
tegra2_osc_freq_detect(void)
{
	bus_space_handle_t	bsh;
	uint32_t		c;
	uint32_t		r=0;
	int			i=0;

	struct {
		uint32_t val;
		uint32_t freq;
	} freq_det_cnts[] = {
		{ 732,  12000000 },
		{ 794,  13000000 },
		{1172,  19200000 },
		{1587,  26000000 },
		{  -1,         0 },
	};

	printf("Measuring...\n");
	bus_space_map(fdtbus_bs_tag,TEGRA2_CLK_RST_PA_BASE, 0x1000, 0, &bsh);

	bus_space_write_4(fdtbus_bs_tag, bsh, TEGRA2_CLK_RST_OSC_FREQ_DET_REG,
			OSC_FREQ_DET_TRIG | 1 );
	do {} while (bus_space_read_4(fdtbus_bs_tag, bsh,
			TEGRA2_CLK_RST_OSC_FREQ_DET_STAT_REG) & OSC_FREQ_DET_BUSY);

	c = bus_space_read_4(fdtbus_bs_tag, bsh, TEGRA2_CLK_RST_OSC_FREQ_DET_STAT_REG);

	while (freq_det_cnts[i].val > 0) {
		if (((freq_det_cnts[i].val - 3) < c) && (c < (freq_det_cnts[i].val + 3)))
			r = freq_det_cnts[i].freq;
		i++;
	}
	printf("c=%u r=%u\n",c,r );
	bus_space_free(fdtbus_bs_tag, bsh, 0x1000);
	return r;
}
#endif

vm_offset_t
initarm_lastaddr(void)
{

	if (fdt_immr_addr(TEGRA2_BASE) != 0)				/* FIXME ???? */
		while (1);

	return (fdt_immr_va - ARM_NOCACHE_KVA_SIZE);
}

void
initarm_gpio_init(void)
{
}

void
initarm_late_init(void)
{
}

#define FDT_DEVMAP_MAX	(1 + 2 + 1 + 1)	/* FIXME */
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
	fdt_devmap[i].pd_va = 0xe0000000;
	fdt_devmap[i].pd_pa = 0x70000000;
	fdt_devmap[i].pd_size = 0x100000;
	fdt_devmap[i].pd_prot = VM_PROT_READ | VM_PROT_WRITE;
	fdt_devmap[i].pd_cache = PTE_NOCACHE;
	i++;

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