
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

/*
 * The psim iobus attachment for the OpenPIC interrupt controller.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>

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
#include <powerpc/psim/iobusvar.h>

#include "pic_if.h"

/*
 * PSIM IOBus interface
 */
static int	openpic_iobus_probe(device_t);
static int	openpic_iobus_attach(device_t);

static device_method_t  openpic_iobus_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		openpic_iobus_probe),
	DEVMETHOD(device_attach,	openpic_iobus_attach),

	/* PIC interface */
	DEVMETHOD(pic_config,		openpic_config),
	DEVMETHOD(pic_dispatch,		openpic_dispatch),
	DEVMETHOD(pic_enable,		openpic_enable),
	DEVMETHOD(pic_eoi,		openpic_eoi),
	DEVMETHOD(pic_ipi,		openpic_ipi),
	DEVMETHOD(pic_mask,		openpic_mask),
	DEVMETHOD(pic_unmask,		openpic_unmask),

	{ 0, 0 }
};

static driver_t openpic_iobus_driver = {
	"openpic",
	openpic_iobus_methods,
	sizeof(struct openpic_softc)
};

DRIVER_MODULE(openpic, iobus, openpic_iobus_driver, openpic_devclass, 0, 0);

static int
openpic_iobus_probe(device_t dev)
{
	struct openpic_softc *sc;
	char *name;

	name = iobus_get_name(dev);
	if (strcmp(name, "interrupt-controller") != 0)
		return (ENXIO);

	/*
	 * The description was already printed out in the nexus
	 * probe, so don't do it again here
	 */
	device_set_desc(dev, OPENPIC_DEVSTR);

	sc = device_get_softc(dev);
	sc->sc_psim = 1;

	return (0);
}

static int
openpic_iobus_attach(device_t dev)
{

	return (openpic_common_attach(dev, 0));
}