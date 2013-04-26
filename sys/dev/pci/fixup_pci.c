
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
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/bus.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

/*
 * Chipset fixups.
 *
 * These routines are invoked during the probe phase for devices which
 * typically don't have specific device drivers, but which require
 * some cleaning up.
 */

static int	fixup_pci_probe(device_t dev);
static void	fixwsc_natoma(device_t dev);
static void	fixc1_nforce2(device_t dev);

static device_method_t fixup_pci_methods[] = {
    /* Device interface */
    DEVMETHOD(device_probe,		fixup_pci_probe),
    DEVMETHOD(device_attach,		bus_generic_attach),
    { 0, 0 }
};

static driver_t fixup_pci_driver = {
    "fixup_pci",
    fixup_pci_methods,
    0,
};

static devclass_t fixup_pci_devclass;

DRIVER_MODULE(fixup_pci, pci, fixup_pci_driver, fixup_pci_devclass, 0, 0);

static int
fixup_pci_probe(device_t dev)
{
    switch (pci_get_devid(dev)) {
    case 0x12378086:		/* Intel 82440FX (Natoma) */
	fixwsc_natoma(dev);
	break;
    case 0x01e010de:		/* nVidia nForce2 */
	fixc1_nforce2(dev);
	break;
    }
    return(ENXIO);
}

static void
fixwsc_natoma(device_t dev)
{
    int		pmccfg;

    pmccfg = pci_read_config(dev, 0x50, 2);
#if defined(SMP)
    if (pmccfg & 0x8000) {
	printf("Correcting Natoma config for SMP\n");
	pmccfg &= ~0x8000;
	pci_write_config(dev, 0x50, pmccfg, 2);
    }
#else
    if ((pmccfg & 0x8000) == 0) {
	printf("Correcting Natoma config for non-SMP\n");
	pmccfg |= 0x8000;
	pci_write_config(dev, 0x50, pmccfg, 2);
    }
#endif
}

/*
 * Set the SYSTEM_IDLE_TIMEOUT to 80 ns on nForce2 systems to work
 * around a hang that is triggered when the CPU generates a very fast
 * CONNECT/HALT cycle sequence.  Specifically, the hang can result in
 * the lapic timer being stopped.
 *
 * This requires changing the value for config register at offset 0x6c
 * for the Host-PCI bridge at bus/dev/function 0/0/0:
 *
 * Chip	Current Value	New Value
 * ----	----------	----------
 * C17	0x1F0FFF01	0x1F01FF01
 * C18D	0x9F0FFF01	0x9F01FF01
 *
 * We do this by always clearing the bits in 0x000e0000.
 *
 * See also: http://lkml.org/lkml/2004/5/3/157
 */
static void
fixc1_nforce2(device_t dev)
{
	uint32_t val;

	if (pci_get_bus(dev) == 0 && pci_get_slot(dev) == 0 &&
	    pci_get_function(dev) == 0) {
		val = pci_read_config(dev, 0x6c, 4);
		if (val & 0x000e0000) {
			printf("Correcting nForce2 C1 CPU disconnect hangs\n");
			val &= ~0x000e0000;
			pci_write_config(dev, 0x6c, val, 4);
		}
	}
}