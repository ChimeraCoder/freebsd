
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
#include <machine/intr.h>
#include <machine/pte.h>
#include <machine/pmap.h>
#include <machine/vmparam.h>

#include <arm/mv/mvreg.h>
#include <arm/mv/mvvar.h>
#include <arm/mv/mvwin.h>

/*
 * Virtual address space layout:
 * -----------------------------
 * 0x0000_0000 - 0xbfff_ffff	: user process
 *
 * 0xc040_0000 - virtual_avail	: kernel reserved (text, data, page tables
 *				: structures, ARM stacks etc.)
 * virtual_avail - 0xefff_ffff	: KVA (virtual_avail is typically < 0xc0a0_0000)
 * 0xf000_0000 - 0xf0ff_ffff	: no-cache allocation area (16MB)
 * 0xf100_0000 - 0xf10f_ffff	: SoC integrated devices registers range (1MB)
 * 0xf110_0000 - 0xf11f_ffff	: PCI-Express I/O space (1MB)
 * 0xf120_0000 - 0xf12f_ffff	: PCI I/O space (1MB)
 * 0xf130_0000 - 0xf52f_ffff	: PCI-Express memory space (64MB)
 * 0xf530_0000 - 0xf92f_ffff	: PCI memory space (64MB)
 * 0xf930_0000 - 0xfffe_ffff	: unused (~108MB)
 * 0xffff_0000 - 0xffff_0fff	: 'high' vectors page (4KB)
 * 0xffff_1000 - 0xffff_1fff	: ARM_TP_ADDRESS/RAS page (4KB)
 * 0xffff_2000 - 0xffff_ffff	: unused (~55KB)
 */


#if 0
int platform_pci_get_irq(u_int bus, u_int slot, u_int func, u_int pin);

/* Static device mappings. */
const struct pmap_devmap pmap_devmap[] = {
	/*
	 * Map the on-board devices VA == PA so that we can access them
	 * with the MMU on or off.
	 */
	{ /* SoC integrated peripherals registers range */
		MV_BASE,
		MV_PHYS_BASE,
		MV_SIZE,
		VM_PROT_READ | VM_PROT_WRITE,
		PTE_NOCACHE,
	},
	{ /* PCIE I/O */
		MV_PCIE_IO_BASE,
		MV_PCIE_IO_PHYS_BASE,
		MV_PCIE_IO_SIZE,
		VM_PROT_READ | VM_PROT_WRITE,
		PTE_NOCACHE,
	},
	{ /* PCIE Memory */
		MV_PCIE_MEM_BASE,
		MV_PCIE_MEM_PHYS_BASE,
		MV_PCIE_MEM_SIZE,
		VM_PROT_READ | VM_PROT_WRITE,
		PTE_NOCACHE,
	},
	{ /* PCI I/O */
		MV_PCI_IO_BASE,
		MV_PCI_IO_PHYS_BASE,
		MV_PCI_IO_SIZE,
		VM_PROT_READ | VM_PROT_WRITE,
		PTE_NOCACHE,
	},
	{ /* PCI Memory */
		MV_PCI_MEM_BASE,
		MV_PCI_MEM_PHYS_BASE,
		MV_PCI_MEM_SIZE,
		VM_PROT_READ | VM_PROT_WRITE,
		PTE_NOCACHE,
	},
	{ /* 7-seg LED */
		MV_DEV_CS0_BASE,
		MV_DEV_CS0_PHYS_BASE,
		MV_DEV_CS0_SIZE,
		VM_PROT_READ | VM_PROT_WRITE,
		PTE_NOCACHE,
	},
	{ 0, 0, 0, 0, 0, }
};

/*
 * The pci_irq_map table consists of 3 columns:
 * - PCI slot number (less than zero means ANY).
 * - PCI IRQ pin (less than zero means ANY).
 * - PCI IRQ (less than zero marks end of table).
 *
 * IRQ number from the first matching entry is used to configure PCI device
 */

/* PCI IRQ Map for DB-88F5281 */
const struct obio_pci_irq_map pci_irq_map[] = {
	{ 7, -1, GPIO2IRQ(12) },
	{ 8, -1, GPIO2IRQ(13) },
	{ 9, -1, GPIO2IRQ(13) },
	{ -1, -1, -1 }
};

/* PCI IRQ Map for DB-88F5182 */
const struct obio_pci_irq_map pci_irq_map[] = {
	{ 7, -1, GPIO2IRQ(0) },
	{ 8, -1, GPIO2IRQ(1) },
	{ 9, -1, GPIO2IRQ(1) },
	{ -1, -1, -1 }
};
#endif

#if 0
/*
 * mv_gpio_config row structure:
 *	<GPIO number>, <GPIO flags>, <GPIO mode>
 *
 * - GPIO pin number (less than zero marks end of table)
 * - GPIO flags:
 *	MV_GPIO_BLINK
 *	MV_GPIO_POLAR_LOW
 *	MV_GPIO_EDGE
 *	MV_GPIO_LEVEL
 * - GPIO mode:
 *	1	- Output, set to HIGH.
 *	0	- Output, set to LOW.
 *	-1	- Input.
 */

/* GPIO Configuration for DB-88F5281 */
const struct gpio_config mv_gpio_config[] = {
	{ 12, MV_GPIO_POLAR_LOW | MV_GPIO_LEVEL, -1 },
	{ 13, MV_GPIO_POLAR_LOW | MV_GPIO_LEVEL, -1 },
	{ -1, -1, -1 }
};

#if 0
/* GPIO Configuration for DB-88F5182 */
const struct gpio_config mv_gpio_config[] = {
	{ 0, MV_GPIO_POLAR_LOW | MV_GPIO_LEVEL, -1 },
	{ 1, MV_GPIO_POLAR_LOW | MV_GPIO_LEVEL, -1 },
	{ -1, -1, -1 }
};
#endif

#endif