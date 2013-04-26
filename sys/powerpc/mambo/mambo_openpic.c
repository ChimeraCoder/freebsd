
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
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/openfirm.h>

#include <machine/bus.h>
#include <machine/intr_machdep.h>
#include <machine/md_var.h>
#include <machine/pio.h>
#include <machine/resource.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <sys/rman.h>

#include <machine/openpicvar.h>

#include "pic_if.h"

/*
 * Mambo interface
 */
static int	openpic_mambo_probe(device_t);
static int	openpic_mambo_attach(device_t);
static int	openpicbus_mambo_probe(device_t dev);
static int	openpicbus_mambo_attach(device_t dev);
static struct resource *openpicbus_alloc_resource(device_t bus, device_t dev,
    int type, int *rid, u_long start, u_long end, u_long count, u_int flags);

static device_method_t  openpicbus_mambo_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		openpicbus_mambo_probe),
	DEVMETHOD(device_attach,	openpicbus_mambo_attach),

	/* Bus interface */
	DEVMETHOD(bus_alloc_resource,	openpicbus_alloc_resource),

	{0,0}
};

struct openpicbus_softc {
	vm_offset_t picaddr;
};

static driver_t openpicbus_mambo_driver = {
	"openpicbus",
	openpicbus_mambo_methods,
	sizeof(struct openpicbus_softc),
};

static device_method_t  openpic_mambo_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		openpic_mambo_probe),
	DEVMETHOD(device_attach,	openpic_mambo_attach),

	/* PIC interface */
	DEVMETHOD(pic_config,		openpic_config),
	DEVMETHOD(pic_dispatch,		openpic_dispatch),
	DEVMETHOD(pic_enable,		openpic_enable),
	DEVMETHOD(pic_eoi,		openpic_eoi),
	DEVMETHOD(pic_ipi,		openpic_ipi),
	DEVMETHOD(pic_mask,		openpic_mask),
	DEVMETHOD(pic_unmask,		openpic_unmask),

	{ 0, 0 },
};

static driver_t openpic_mambo_driver = {
	"openpic",
	openpic_mambo_methods,
	sizeof(struct openpic_softc),
};

static devclass_t openpicbus_devclass;

DRIVER_MODULE(openpicbus, nexus, openpicbus_mambo_driver, 
    openpicbus_devclass, 0, 0);
DRIVER_MODULE(openpic, openpicbus, openpic_mambo_driver, 
    openpic_devclass, 0, 0);

static int
openpicbus_mambo_probe(device_t dev)
{
	const char *type = ofw_bus_get_type(dev);

	if (type == NULL || strcmp(type, "open-pic") != 0)
                return (ENXIO);

	device_set_desc(dev, "Mambo OpenPIC Container");

	return (0);
}

static int
openpicbus_mambo_attach(device_t dev)
{
	uint64_t picaddr;
	phandle_t nexus;
	struct openpicbus_softc *sc;

	sc = device_get_softc(dev);

	nexus = OF_parent(ofw_bus_get_node(dev));

	OF_getprop(nexus,"platform-open-pic",
	    &picaddr,sizeof(picaddr));

	sc->picaddr = picaddr;

	device_add_child(dev,"openpic",-1);

	return (bus_generic_attach(dev));
}

static struct resource *
openpicbus_alloc_resource(device_t bus, device_t dev, int type, int *rid,
		   u_long start, u_long end, u_long count, u_int flags)
{
	struct openpicbus_softc *sc;

	sc = device_get_softc(bus);

	count = 0x40000;
	start = sc->picaddr;
	end = start + count;

	return (bus_alloc_resource(bus, type, rid, start, end, count, flags));
}

static int
openpic_mambo_probe(device_t dev)
{

	device_set_desc(dev, OPENPIC_DEVSTR);
	return (0);
}

static int
openpic_mambo_attach(device_t dev)
{
 
	return (openpic_common_attach(dev,
	    ofw_bus_get_node(device_get_parent(dev))));
}