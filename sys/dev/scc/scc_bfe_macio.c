
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

#include <dev/ofw/ofw_bus.h>

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/scc/scc_bfe.h>

#define	MACIO_REGSHFT	4
#define	MACIO_RCLK	230400

static int
scc_macio_probe(device_t dev)
{
	struct scc_softc *sc;
	const char *nm;

	sc = device_get_softc(dev);
	nm = ofw_bus_get_name(dev);
	if (!strcmp(nm, "escc")) {
		device_set_desc(dev, "Zilog Z8530 dual channel SCC");
		sc->sc_class = &scc_z8530_class;
		return (scc_bfe_probe(dev, MACIO_REGSHFT, MACIO_RCLK, 0));
	}
	return (ENXIO);
}

static int
scc_macio_attach(device_t dev)
{

	return (scc_bfe_attach(dev, 3));
}

static device_method_t scc_macio_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		scc_macio_probe),
	DEVMETHOD(device_attach,	scc_macio_attach),
	DEVMETHOD(device_detach,	scc_bfe_detach),

	DEVMETHOD(bus_alloc_resource,	scc_bus_alloc_resource),
	DEVMETHOD(bus_release_resource,	scc_bus_release_resource),
	DEVMETHOD(bus_get_resource,	scc_bus_get_resource),
	DEVMETHOD(bus_read_ivar,	scc_bus_read_ivar),
	DEVMETHOD(bus_setup_intr,	scc_bus_setup_intr),
	DEVMETHOD(bus_teardown_intr,	scc_bus_teardown_intr),

	DEVMETHOD_END
};

static driver_t scc_macio_driver = {
	scc_driver_name,
	scc_macio_methods,
	sizeof(struct scc_softc),
};

DRIVER_MODULE(scc, macio, scc_macio_driver, scc_devclass, 0, 0);