
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

#include <isa/isavar.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>

static int uart_acpi_probe(device_t dev);

static device_method_t uart_acpi_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_acpi_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	DEVMETHOD(device_resume,	uart_bus_resume),
	{ 0, 0 }
};

static driver_t uart_acpi_driver = {
	uart_driver_name,
	uart_acpi_methods,
	sizeof(struct uart_softc),
};

static struct isa_pnp_id acpi_ns8250_ids[] = {
	{0x0005d041, "Standard PC COM port"},		/* PNP0500 */
	{0x0105d041, "16550A-compatible COM port"},	/* PNP0501 */
	{0x0205d041, "Multiport serial device (non-intelligent 16550)"}, /* PNP0502 */
	{0x1005d041, "Generic IRDA-compatible device"},	/* PNP0510 */
	{0x1105d041, "Generic IRDA-compatible device"},	/* PNP0511 */
	{0x04f0235c, "Wacom Tablet PC Screen"},		/* WACF004 */
	{0xe502aa1a, "Wacom Tablet at FuS Lifebook T"},	/* FUJ02E5 */
	{0}
};

static int
uart_acpi_probe(device_t dev)
{
	struct uart_softc *sc;
	device_t parent;

	parent = device_get_parent(dev);
	sc = device_get_softc(dev);

	if (!ISA_PNP_PROBE(parent, dev, acpi_ns8250_ids)) {
		sc->sc_class = &uart_ns8250_class;
		return (uart_bus_probe(dev, 0, 0, 0, 0));
	}

	/* Add checks for non-ns8250 IDs here. */
	return (ENXIO);
}

DRIVER_MODULE(uart, acpi, uart_acpi_driver, uart_devclass, 0, 0);