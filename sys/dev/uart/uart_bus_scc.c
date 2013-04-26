
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

#include <dev/scc/scc_bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>

static int uart_scc_attach(device_t dev);
static int uart_scc_probe(device_t dev);

static device_method_t uart_scc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_scc_probe),
	DEVMETHOD(device_attach,	uart_scc_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	/* Serdev interface */
	DEVMETHOD(serdev_ihand,		uart_bus_ihand),
	DEVMETHOD(serdev_sysdev,	uart_bus_sysdev),
	{ 0, 0 }
};

static driver_t uart_scc_driver = {
	uart_driver_name,
	uart_scc_methods,
	sizeof(struct uart_softc),
};

static int
uart_scc_attach(device_t dev)
{
	device_t parent;
	struct uart_softc *sc;
	uintptr_t mtx;

	parent = device_get_parent(dev);
	sc = device_get_softc(dev);

	if (BUS_READ_IVAR(parent, dev, SCC_IVAR_HWMTX, &mtx))
		return (ENXIO);
	sc->sc_hwmtx = (struct mtx *)(void *)mtx;
	return (uart_bus_attach(dev));
}

static int
uart_scc_probe(device_t dev)
{
	device_t parent;
	struct uart_softc *sc;
	uintptr_t ch, cl, md, rs;

	parent = device_get_parent(dev);
	sc = device_get_softc(dev);

	if (BUS_READ_IVAR(parent, dev, SCC_IVAR_MODE, &md) ||
	    BUS_READ_IVAR(parent, dev, SCC_IVAR_CLASS, &cl))
		return (ENXIO);
	if (md != SCC_MODE_ASYNC)
		return (ENXIO);
	switch (cl) {
	case SCC_CLASS_QUICC:
		sc->sc_class = &uart_quicc_class;
		break;
	case SCC_CLASS_SAB82532:
		sc->sc_class = &uart_sab82532_class;
		break;
	case SCC_CLASS_Z8530:
		sc->sc_class = &uart_z8530_class;
		break;
	default:
		return (ENXIO);
	}
	if (BUS_READ_IVAR(parent, dev, SCC_IVAR_CHANNEL, &ch) ||
	    BUS_READ_IVAR(parent, dev, SCC_IVAR_CLOCK, &cl) ||
	    BUS_READ_IVAR(parent, dev, SCC_IVAR_REGSHFT, &rs))
		return (ENXIO);

	return (uart_bus_probe(dev, rs, cl, 0, ch));
}

DRIVER_MODULE(uart, scc, uart_scc_driver, uart_devclass, 0, 0);