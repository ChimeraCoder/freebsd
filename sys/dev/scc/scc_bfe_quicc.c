
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

#define	__RMAN_RESOURCE_VISIBLE

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <dev/quicc/quicc_bus.h>

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/scc/scc_bfe.h>

static int
scc_quicc_probe(device_t dev)
{
	device_t parent;
	struct scc_softc *sc;
	uintptr_t devtype, rclk;
	int error;

	parent = device_get_parent(dev);

	error = BUS_READ_IVAR(parent, dev, QUICC_IVAR_DEVTYPE, &devtype);
	if (error)
		return (error);
	if (devtype != QUICC_DEVTYPE_SCC)
		return (ENXIO);

	device_set_desc(dev, "QUICC quad channel SCC");

	sc = device_get_softc(dev);
	sc->sc_class = &scc_quicc_class;
	if (BUS_READ_IVAR(parent, dev, QUICC_IVAR_BRGCLK, &rclk))
		rclk = 0;
	return (scc_bfe_probe(dev, 0, rclk, 0));
}

static int
scc_quicc_attach(device_t dev)
{

	return (scc_bfe_attach(dev, 0));
}

static device_method_t scc_quicc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		scc_quicc_probe),
	DEVMETHOD(device_attach,	scc_quicc_attach),
	DEVMETHOD(device_detach,	scc_bfe_detach),

	DEVMETHOD(bus_alloc_resource,	scc_bus_alloc_resource),
	DEVMETHOD(bus_release_resource,	scc_bus_release_resource),
	DEVMETHOD(bus_get_resource,	scc_bus_get_resource),
	DEVMETHOD(bus_read_ivar,	scc_bus_read_ivar),
	DEVMETHOD(bus_setup_intr,	scc_bus_setup_intr),
	DEVMETHOD(bus_teardown_intr,	scc_bus_teardown_intr),

	DEVMETHOD_END
};

static driver_t scc_quicc_driver = {
	scc_driver_name,
	scc_quicc_methods,
	sizeof(struct scc_softc),
};

DRIVER_MODULE(scc, quicc, scc_quicc_driver, scc_devclass, 0, 0);