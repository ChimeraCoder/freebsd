
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
	/* Device interface */	DEVMETHOD(device_probe,		uart_s3c2410_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t uart_s3c2410_driver = {
	uart_driver_name,
	uart_s3c2410_methods,
	sizeof(struct uart_softc),
};

extern struct uart_class uart_s3c2410_class;
extern SLIST_HEAD(uart_devinfo_list, uart_devinfo) uart_sysdevs;
static int
uart_s3c2410_probe(device_t dev)
{
	struct uart_devinfo *sysdev;
	struct uart_softc *sc;
	int unit;

	sc = device_get_softc(dev);
	sc->sc_class = &uart_s3c2410_class;

	unit = device_get_unit(dev);
	sysdev = SLIST_FIRST(&uart_sysdevs);
	if (S3C24X0_UART_BASE(unit) == sysdev->bas.bsh) {
		sc->sc_sysdev = sysdev;
		bcopy(&sc->sc_sysdev->bas, &sc->sc_bas, sizeof(sc->sc_bas));
	}
	return(uart_bus_probe(dev, 0, 0, 0, unit));
}

DRIVER_MODULE(uart, s3c24x0, uart_s3c2410_driver, uart_devclass, 0, 0);