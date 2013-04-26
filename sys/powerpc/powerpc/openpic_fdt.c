
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/intr_machdep.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <machine/openpicvar.h>

#include "pic_if.h"

static int openpic_fdt_probe(device_t);
static int openpic_fdt_attach(device_t);

static device_method_t openpic_fdt_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		openpic_fdt_probe),
	DEVMETHOD(device_attach,	openpic_fdt_attach),

	/* PIC interface */
	DEVMETHOD(pic_bind,		openpic_bind),
	DEVMETHOD(pic_config,		openpic_config),
	DEVMETHOD(pic_dispatch,		openpic_dispatch),
	DEVMETHOD(pic_enable,		openpic_enable),
	DEVMETHOD(pic_eoi,		openpic_eoi),
	DEVMETHOD(pic_ipi,		openpic_ipi),
	DEVMETHOD(pic_mask,		openpic_mask),
	DEVMETHOD(pic_unmask,		openpic_unmask),

	{ 0, 0 },
};

static driver_t openpic_fdt_driver = {
	"openpic",
	openpic_fdt_methods,
	sizeof(struct openpic_softc)
};

DRIVER_MODULE(openpic, simplebus, openpic_fdt_driver, openpic_devclass, 0, 0);

static int
openpic_fdt_probe(device_t dev)
{

	if (!ofw_bus_is_compatible(dev, "chrp,open-pic"))
		return (ENXIO);
		
	device_set_desc(dev, OPENPIC_DEVSTR);
	return (BUS_PROBE_DEFAULT);
}

static int
openpic_fdt_attach(device_t dev)
{

	return (openpic_common_attach(dev, ofw_bus_get_node(dev)));
}