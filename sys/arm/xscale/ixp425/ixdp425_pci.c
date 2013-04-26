
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

#define _ARM32_BUS_DMA_PRIVATE
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/malloc.h>
#include <sys/rman.h>

#include <dev/pci/pcivar.h>

#include <machine/bus.h>
#include <machine/intr.h>

#include <arm/xscale/ixp425/ixp425reg.h>
#include <arm/xscale/ixp425/ixp425var.h>
#include <arm/xscale/ixp425/ixp425_intr.h>
#include <arm/xscale/ixp425/ixdp425reg.h>

void
ixp425_md_attach(device_t dev)
{
	struct ixp425_softc *sc = device_get_softc(device_get_parent(dev));
	struct ixppcib_softc *pci_sc = device_get_softc(dev);
	uint32_t reg;

	
	/* PCI Reset Assert */
	reg = GPIO_CONF_READ_4(sc, IXP425_GPIO_GPOUTR);
	reg &= ~(1U << GPIO_PCI_RESET);
	GPIO_CONF_WRITE_4(sc, IXP425_GPIO_GPOUTR, reg);

	/* PCI Clock Disable */
	reg = GPIO_CONF_READ_4(sc, IXP425_GPIO_GPCLKR);
	reg &= ~GPCLKR_MUX14;
	GPIO_CONF_WRITE_4(sc, IXP425_GPIO_GPCLKR, reg);

	/*
	 * set GPIO Direction
	 *	Output: PCI_CLK, PCI_RESET
	 *	Input:  PCI_INTA, PCI_INTB, PCI_INTC, PCI_INTD
	 */
	reg = GPIO_CONF_READ_4(sc, IXP425_GPIO_GPOER);
	reg &= ~(1U << GPIO_PCI_CLK);
	reg &= ~(1U << GPIO_PCI_RESET);
	reg |= ((1U << GPIO_PCI_INTA) | (1U << GPIO_PCI_INTB) |
		(1U << GPIO_PCI_INTC) | (1U << GPIO_PCI_INTD));
	GPIO_CONF_WRITE_4(sc, IXP425_GPIO_GPOER, reg);

	/*
	 * Set GPIO interrupt type
	 * 	PCI_INT_A, PCI_INTB, PCI_INT_C, PCI_INT_D: Active Low
	 */
	reg = GPIO_CONF_READ_4(sc, GPIO_TYPE_REG(GPIO_PCI_INTA));
	reg &= ~GPIO_TYPE(GPIO_PCI_INTA, GPIO_TYPE_MASK);
	reg |= GPIO_TYPE(GPIO_PCI_INTA, GPIO_TYPE_ACT_LOW);
	GPIO_CONF_WRITE_4(sc, GPIO_TYPE_REG(GPIO_PCI_INTA), reg);

	reg = GPIO_CONF_READ_4(sc, GPIO_TYPE_REG(GPIO_PCI_INTB));
	reg &= ~GPIO_TYPE(GPIO_PCI_INTB, GPIO_TYPE_MASK);
	reg |= GPIO_TYPE(GPIO_PCI_INTB, GPIO_TYPE_ACT_LOW);
	GPIO_CONF_WRITE_4(sc, GPIO_TYPE_REG(GPIO_PCI_INTB), reg);

	reg = GPIO_CONF_READ_4(sc, GPIO_TYPE_REG(GPIO_PCI_INTC));
	reg &= ~GPIO_TYPE(GPIO_PCI_INTC, GPIO_TYPE_MASK);
	reg |= GPIO_TYPE(GPIO_PCI_INTC, GPIO_TYPE_ACT_LOW);
	GPIO_CONF_WRITE_4(sc, GPIO_TYPE_REG(GPIO_PCI_INTC), reg);

	reg = GPIO_CONF_READ_4(sc, GPIO_TYPE_REG(GPIO_PCI_INTD));
	reg &= ~GPIO_TYPE(GPIO_PCI_INTD, GPIO_TYPE_MASK);
	reg |= GPIO_TYPE(GPIO_PCI_INTD, GPIO_TYPE_ACT_LOW);
	GPIO_CONF_WRITE_4(sc, GPIO_TYPE_REG(GPIO_PCI_INTD), reg);

	/* clear ISR */
	GPIO_CONF_WRITE_4(sc, IXP425_GPIO_GPISR,
			  (1U << GPIO_PCI_INTA) | (1U << GPIO_PCI_INTB) |
			  (1U << GPIO_PCI_INTC) | (1U << GPIO_PCI_INTD));

	/* wait 1ms to satisfy "minimum reset assertion time" of the PCI spec */
	DELAY(1000);
	reg = GPIO_CONF_READ_4(sc, IXP425_GPIO_GPCLKR);
	GPIO_CONF_WRITE_4(sc, IXP425_GPIO_GPCLKR, reg |
		(0xf << GPCLKR_CLK0DC_SHIFT) | (0xf << GPCLKR_CLK0TC_SHIFT));

	/* PCI Clock Enable */
	reg = GPIO_CONF_READ_4(sc, IXP425_GPIO_GPCLKR);
	reg |= GPCLKR_MUX14;
	GPIO_CONF_WRITE_4(sc, IXP425_GPIO_GPCLKR, reg | GPCLKR_MUX14);

	/*
	 * wait 100us to satisfy "minimum reset assertion time from clock stable
	 * requirement of the PCI spec
	 */
	DELAY(100);
        /* PCI Reset deassert */
	reg = GPIO_CONF_READ_4(sc, IXP425_GPIO_GPOUTR);
	reg |= 1U << GPIO_PCI_RESET;
	GPIO_CONF_WRITE_4(sc, IXP425_GPIO_GPOUTR, reg | (1U << GPIO_PCI_RESET));
	pci_sc->sc_irq_rman.rm_type = RMAN_ARRAY;
	pci_sc->sc_irq_rman.rm_descr = "IXP425 PCI IRQs";
	CTASSERT(PCI_INT_D < PCI_INT_A);
	/* XXX this overlaps the irq's setup in ixp425_attach */
	if (rman_init(&pci_sc->sc_irq_rman) != 0 ||
	    rman_manage_region(&pci_sc->sc_irq_rman, PCI_INT_D, PCI_INT_A) != 0)
		panic("ixp425_md_attach: failed to set up IRQ rman");
}

#define	IXP425_MAX_DEV	5
#define	IXP425_MAX_LINE	4

int
ixp425_md_route_interrupt(device_t bridge, device_t device, int pin)
{
	static int ixp425_pci_table[IXP425_MAX_DEV][IXP425_MAX_LINE] = {
		{PCI_INT_A, PCI_INT_B, PCI_INT_C, PCI_INT_D},
		{PCI_INT_B, PCI_INT_C, PCI_INT_D, PCI_INT_A},
		{PCI_INT_C, PCI_INT_D, PCI_INT_A, PCI_INT_B},
		{PCI_INT_D, PCI_INT_A, PCI_INT_B, PCI_INT_C},
		/* NB: for optional USB controller on Gateworks Avila */
		{PCI_INT_A, PCI_INT_B, PCI_INT_C, PCI_INT_D},
	};
	int dev;
	
	dev = pci_get_slot(device);
	if (bootverbose)
		device_printf(bridge, "routing pin %d for %s\n", pin,
		    device_get_nameunit(device));
	if (pin >= 1 && pin <= IXP425_MAX_LINE &&
	    dev >= 1 && dev <= IXP425_MAX_DEV) {
		return (ixp425_pci_table[dev - 1][pin - 1]);
	} else
		printf("ixppcib: no mapping for %d/%d/%d\n",
			pci_get_bus(device), dev, pci_get_function(device));

	return (-1);
}