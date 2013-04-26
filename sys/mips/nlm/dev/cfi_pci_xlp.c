
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
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/pci/pcivar.h>

#include <dev/cfi/cfi_var.h>

#include <mips/nlm/hal/haldefs.h>
#include <mips/nlm/hal/iomap.h>

static int cfi_xlp_probe(device_t dev);

static device_method_t cfi_xlp_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		cfi_xlp_probe),
	DEVMETHOD(device_attach,	cfi_attach),
	DEVMETHOD(device_detach,	cfi_detach),
	DEVMETHOD_END
};

static driver_t cfi_xlp_driver = {
	cfi_driver_name,
	cfi_xlp_methods,
	sizeof(struct cfi_softc),
};

static int
cfi_xlp_probe(device_t dev)
{

	if (pci_get_vendor(dev) != PCI_VENDOR_NETLOGIC ||
	    pci_get_device(dev) != PCI_DEVICE_ID_NLM_NOR)
		return (ENXIO);

	device_set_desc(dev, "Netlogic XLP NOR Bus");
	return (cfi_probe(dev));
}

DRIVER_MODULE(cfi_xlp, pci, cfi_xlp_driver, cfi_devclass, 0, 0);