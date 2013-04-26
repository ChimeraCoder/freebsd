
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
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/rman.h>
#include <sys/tty.h>
#include <machine/bus.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>
#include <dev/quicc/quicc_bfe.h>

static int quicc_fdt_probe(device_t dev);

static device_method_t quicc_fdt_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		quicc_fdt_probe),
	DEVMETHOD(device_attach,	quicc_bfe_attach),
	DEVMETHOD(device_detach,	quicc_bfe_detach),

	DEVMETHOD(bus_alloc_resource,	quicc_bus_alloc_resource),
	DEVMETHOD(bus_release_resource,	quicc_bus_release_resource),
	DEVMETHOD(bus_get_resource,	quicc_bus_get_resource),
	DEVMETHOD(bus_read_ivar,	quicc_bus_read_ivar),
	DEVMETHOD(bus_setup_intr,	quicc_bus_setup_intr),
	DEVMETHOD(bus_teardown_intr,	quicc_bus_teardown_intr),

	DEVMETHOD_END
};

static driver_t quicc_fdt_driver = {
	quicc_driver_name,
	quicc_fdt_methods,
	sizeof(struct quicc_softc),
};

static int
quicc_fdt_probe(device_t dev)
{
	phandle_t par;
	pcell_t clock;

	if (!ofw_bus_is_compatible(dev, "fsl,cpm2"))
		return (ENXIO);

	par = OF_parent(ofw_bus_get_node(dev));
	if (OF_getprop(par, "bus-frequency", &clock, sizeof(clock)) <= 0)
		clock = 0;

	return (quicc_bfe_probe(dev, (uintptr_t)clock));
}

DRIVER_MODULE(quicc, simplebus, quicc_fdt_driver, quicc_devclass, 0, 0);