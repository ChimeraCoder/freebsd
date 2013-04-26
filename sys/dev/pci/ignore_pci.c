
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
 * 'Ignore' driver - eats devices that show up errnoeously on PCI
 * but shouldn't ever be listed or handled by a driver.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <dev/pci/pcivar.h>

static int	ignore_pci_probe(device_t dev);

static device_method_t ignore_pci_methods[] = {
    /* Device interface */
    DEVMETHOD(device_probe,		ignore_pci_probe),
    DEVMETHOD(device_attach,		bus_generic_attach),
    { 0, 0 }
};

static driver_t ignore_pci_driver = {
    "ignore_pci",
    ignore_pci_methods,
    0,
};

static devclass_t ignore_pci_devclass;

DRIVER_MODULE(ignore_pci, pci, ignore_pci_driver, ignore_pci_devclass, 0, 0);

static int
ignore_pci_probe(device_t dev)
{
    switch (pci_get_devid(dev)) {
    case 0x10001042ul:	/* SMC 37C665 */
	device_set_desc(dev, "ignored");
	device_quiet(dev);
	return(-10000);
    }
    return(ENXIO);
}