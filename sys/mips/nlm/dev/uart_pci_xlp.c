
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

#include <mips/nlm/hal/haldefs.h>
#include <mips/nlm/hal/iomap.h>
#include <mips/nlm/hal/uart.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>

static int uart_soc_probe(device_t dev);

static device_method_t uart_soc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_soc_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),

	DEVMETHOD_END
};

static driver_t uart_soc_driver = {
	uart_driver_name,
	uart_soc_methods,
	sizeof(struct uart_softc),
};

static int
uart_soc_probe(device_t dev)
{
	struct uart_softc *sc;

	if (pci_get_vendor(dev) != PCI_VENDOR_NETLOGIC ||
	    pci_get_device(dev) != PCI_DEVICE_ID_NLM_UART)
		return (ENXIO);

	sc = device_get_softc(dev);
	sc->sc_class = &uart_ns8250_class;
	device_set_desc(dev, "Netlogic SoC UART");
	return (uart_bus_probe(dev, 2, XLP_IO_CLK, 0, 0));
}

DRIVER_MODULE(uart_soc, pci, uart_soc_driver, uart_devclass, 0, 0);