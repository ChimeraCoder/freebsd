
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

#include "opt_acpi.h"

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/rman.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include <dev/acpica/acpivar.h>
#include <dev/acpica/acpi_pcibvar.h>

#include <machine/pci_cfgreg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcib_private.h>
#include "pcib_if.h"

/* Hooks for the ACPI CA debugging infrastructure. */
#define _COMPONENT	ACPI_BUS
ACPI_MODULE_NAME("PCI_PCI")

struct acpi_pcib_softc {
    struct pcib_softc	ap_pcibsc;
    ACPI_HANDLE		ap_handle;
    ACPI_BUFFER		ap_prt;		/* interrupt routing table */
};

struct acpi_pcib_lookup_info {
    UINT32		address;
    ACPI_HANDLE		handle;
};

static int		acpi_pcib_pci_probe(device_t bus);
static int		acpi_pcib_pci_attach(device_t bus);
static int		acpi_pcib_read_ivar(device_t dev, device_t child,
			    int which, uintptr_t *result);
static int		acpi_pcib_pci_route_interrupt(device_t pcib,
			    device_t dev, int pin);

static device_method_t acpi_pcib_pci_methods[] = {
    /* Device interface */
    DEVMETHOD(device_probe,		acpi_pcib_pci_probe),
    DEVMETHOD(device_attach,		acpi_pcib_pci_attach),

    /* Bus interface */
    DEVMETHOD(bus_read_ivar,		acpi_pcib_read_ivar),

    /* pcib interface */
    DEVMETHOD(pcib_route_interrupt,	acpi_pcib_pci_route_interrupt),
    DEVMETHOD(pcib_power_for_sleep,	acpi_pcib_power_for_sleep),

    DEVMETHOD_END
};

static devclass_t pcib_devclass;

DEFINE_CLASS_1(pcib, acpi_pcib_pci_driver, acpi_pcib_pci_methods,
    sizeof(struct acpi_pcib_softc), pcib_driver);
DRIVER_MODULE(acpi_pcib, pci, acpi_pcib_pci_driver, pcib_devclass, 0, 0);
MODULE_DEPEND(acpi_pcib, acpi, 1, 1, 1);

static int
acpi_pcib_pci_probe(device_t dev)
{

    if (pci_get_class(dev) != PCIC_BRIDGE ||
	pci_get_subclass(dev) != PCIS_BRIDGE_PCI ||
	acpi_disabled("pci"))
	return (ENXIO);
    if (acpi_get_handle(dev) == NULL)
	return (ENXIO);
    if (pci_cfgregopen() == 0)
	return (ENXIO);

    device_set_desc(dev, "ACPI PCI-PCI bridge");
    return (-1000);
}

static int
acpi_pcib_pci_attach(device_t dev)
{
    struct acpi_pcib_softc *sc;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    pcib_attach_common(dev);
    sc = device_get_softc(dev);
    sc->ap_handle = acpi_get_handle(dev);
    return (acpi_pcib_attach(dev, &sc->ap_prt, sc->ap_pcibsc.secbus));
}

static int
acpi_pcib_read_ivar(device_t dev, device_t child, int which, uintptr_t *result)
{
    struct acpi_pcib_softc *sc = device_get_softc(dev);

    switch (which) {
    case ACPI_IVAR_HANDLE:
	*result = (uintptr_t)sc->ap_handle;
	return (0);
    }
    return (pcib_read_ivar(dev, child, which, result));
}

static int
acpi_pcib_pci_route_interrupt(device_t pcib, device_t dev, int pin)
{
    struct acpi_pcib_softc *sc;

    sc = device_get_softc(pcib);

    /*
     * If we don't have a _PRT, fall back to the swizzle method
     * for routing interrupts.
     */
    if (sc->ap_prt.Pointer == NULL)
	return (pcib_route_interrupt(pcib, dev, pin));
    else
	return (acpi_pcib_route_interrupt(pcib, dev, pin, &sc->ap_prt));
}