
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

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>
#include <dev/uart/uart_cpu.h>

#include <dev/ic/ns16550.h>

#include <arm/xscale/pxa/pxavar.h>
#include <arm/xscale/pxa/pxareg.h>

#include "uart_if.h"

#define	PXA_UART_UUE	0x40	/* UART Unit Enable */

static int uart_pxa_probe(device_t dev);

static device_method_t uart_pxa_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_pxa_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t uart_pxa_driver = {
	uart_driver_name,
	uart_pxa_methods,
	sizeof(struct uart_softc),
};

static int
uart_pxa_probe(device_t dev)
{
	bus_space_handle_t	base;
	struct			uart_softc *sc;

	base = (bus_space_handle_t)pxa_get_base(dev);
#ifdef QEMU_WORKAROUNDS
	/*
	 * QEMU really exposes only the first uart unless
	 * you specify several of them in the configuration.
	 * Otherwise all the rest of UARTs stay unconnected,
	 * which causes problems in the ns16550 attach routine.
	 * Unfortunately, even if you provide qemu with 4 uarts
	 * on the command line, it has a bug where it segfaults
	 * trying to enable bluetooth on the HWUART.  So we just
	 * allow the FFUART to be attached.
	 * Also, don't check the UUE (UART Unit Enable) bit, as
	 * the gumstix bootloader doesn't set it.
	 */
	if (base != PXA2X0_FFUART_BASE)
		return (ENXIO);
#else
	/* Check to see if the enable bit's on. */
	if ((bus_space_read_4(obio_tag, base,
	    (REG_IER << 2)) & PXA_UART_UUE) == 0)
		return (ENXIO);
#endif
	sc = device_get_softc(dev);
	sc->sc_class = &uart_ns8250_class;

	return(uart_bus_probe(dev, 2, PXA2X0_COM_FREQ, 0, 0));
}

DRIVER_MODULE(uart, pxa, uart_pxa_driver, uart_devclass, 0, 0);