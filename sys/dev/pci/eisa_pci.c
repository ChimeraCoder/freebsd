
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

/*
 * PCI:EISA bridge support
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

static int	eisab_probe(device_t dev);
static int	eisab_attach(device_t dev);

static device_method_t eisab_methods[] = {
    /* Device interface */
    DEVMETHOD(device_probe,		eisab_probe),
    DEVMETHOD(device_attach,		eisab_attach),
    DEVMETHOD(device_shutdown,		bus_generic_shutdown),
    DEVMETHOD(device_suspend,		bus_generic_suspend),
    DEVMETHOD(device_resume,		bus_generic_resume),

    /* Bus interface */
    DEVMETHOD(bus_alloc_resource,	bus_generic_alloc_resource),
    DEVMETHOD(bus_release_resource,	bus_generic_release_resource),
    DEVMETHOD(bus_activate_resource,	bus_generic_activate_resource),
    DEVMETHOD(bus_deactivate_resource,	bus_generic_deactivate_resource),
    DEVMETHOD(bus_setup_intr,		bus_generic_setup_intr),
    DEVMETHOD(bus_teardown_intr,	bus_generic_teardown_intr),

    DEVMETHOD_END
};

static driver_t eisab_driver = {
    "eisab",
    eisab_methods,
    0,
};

static devclass_t eisab_devclass;

DRIVER_MODULE(eisab, pci, eisab_driver, eisab_devclass, 0, 0);

static int
eisab_probe(device_t dev)
{
    int		matched = 0;

    /*
     * Generic match by class/subclass.
     */
    if ((pci_get_class(dev) == PCIC_BRIDGE) &&
	(pci_get_subclass(dev) == PCIS_BRIDGE_EISA))
	matched = 1;

    /*
     * Some bridges don't correctly report their class.
     */
    switch (pci_get_devid(dev)) {
    case 0x04828086:		/* may show up as PCI-HOST or 0:0 */
	matched = 1;
	break;
    default:
	break;
    }
    
    if (matched) {
	device_set_desc(dev, "PCI-EISA bridge");
	return(-10000);
    }
    return(ENXIO);
}

static int
eisab_attach(device_t dev)
{
    /*
     * Attach an EISA bus.  Note that we can only have one EISA bus.
     */
    if (!devclass_get_device(devclass_find("eisa"), 0))
	device_add_child(dev, "eisa", -1);

    /*
     * Attach an ISA bus as well, since the EISA bus may have ISA
     * cards installed, and we may have no EISA support in the system.
     */
    if (!devclass_get_device(devclass_find("isa"), 0))
	device_add_child(dev, "isa", -1);

    bus_generic_attach(dev);

    return(0);
}