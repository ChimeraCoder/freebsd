
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
#include <machine/fdt.h>

#include <arm/mv/mvreg.h>
#include <arm/mv/mvvar.h>
#include <arm/mv/mvwin.h>

#if 0
extern const struct obio_pci_irq_map pci_irq_map[];
const struct obio_pci mv_pci_info[] = {
	{ MV_TYPE_PCIE,
		MV_PCIE_BASE,	MV_PCIE_SIZE,
		MV_PCIE_IO_BASE, MV_PCIE_IO_SIZE,	4, 0x51,
		MV_PCIE_MEM_BASE, MV_PCIE_MEM_SIZE,	4, 0x59,
		NULL, MV_INT_PEX0
	},

	{ MV_TYPE_PCI,
		MV_PCI_BASE, MV_PCI_SIZE,
		MV_PCI_IO_BASE, MV_PCI_IO_SIZE,		3, 0x51,
		MV_PCI_MEM_BASE, MV_PCI_MEM_SIZE,	3, 0x59,
		pci_irq_map, -1
	},

	{ 0, 0, 0 }
};
#endif

struct resource_spec mv_gpio_res[] = {
	{ SYS_RES_MEMORY,	0,	RF_ACTIVE },
	{ SYS_RES_IRQ,		0,	RF_ACTIVE },
	{ SYS_RES_IRQ,		1,	RF_ACTIVE },
	{ SYS_RES_IRQ,		2,	RF_ACTIVE },
	{ SYS_RES_IRQ,		3,	RF_ACTIVE },
	{ -1, 0 }
};

const struct decode_win idma_win_tbl[] = {
	{ 0 },
};
const struct decode_win *idma_wins = idma_win_tbl;
int idma_wins_no = 0;

uint32_t
get_tclk(void)
{
	uint32_t sar;

	/*
	 * On Orion TCLK is can be configured to 150 MHz or 166 MHz.
	 * Current setting is read from Sample At Reset register.
	 */
	/* XXX MPP addr should be retrieved from the DT */
	sar = bus_space_read_4(fdtbus_bs_tag, MV_MPP_BASE, SAMPLE_AT_RESET);
	sar = (sar & TCLK_MASK) >> TCLK_SHIFT;
	switch (sar) {
	case 1:
		return (TCLK_150MHZ);
	case 2:
		return (TCLK_166MHZ);
	default:
		panic("Unknown TCLK settings!");
	}
}