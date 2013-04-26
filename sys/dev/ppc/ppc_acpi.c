
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

#include "opt_isa.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>

#include <isa/isareg.h>
#include <isa/isavar.h>

#include <dev/ppbus/ppbconf.h>
#include <dev/ppbus/ppb_msq.h>
#include <dev/ppc/ppcvar.h>
#include <dev/ppc/ppcreg.h>

#include "ppbus_if.h"

static int ppc_acpi_probe(device_t dev);

int ppc_isa_attach(device_t dev);
int ppc_isa_write(device_t, char *, int, int);

static device_method_t ppc_acpi_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		ppc_acpi_probe),
#ifdef DEV_ISA
	DEVMETHOD(device_attach,	ppc_isa_attach),
#else
	DEVMETHOD(device_attach,	ppc_attach),
#endif
	DEVMETHOD(device_detach,	ppc_detach),

	/* bus interface */
	DEVMETHOD(bus_read_ivar,	ppc_read_ivar),
	DEVMETHOD(bus_write_ivar,	ppc_write_ivar),
	DEVMETHOD(bus_alloc_resource,	ppc_alloc_resource),
	DEVMETHOD(bus_release_resource,	ppc_release_resource),

	/* ppbus interface */
	DEVMETHOD(ppbus_io,		ppc_io),
	DEVMETHOD(ppbus_exec_microseq,	ppc_exec_microseq),
	DEVMETHOD(ppbus_reset_epp,	ppc_reset_epp),
	DEVMETHOD(ppbus_setmode,	ppc_setmode),
	DEVMETHOD(ppbus_ecp_sync,	ppc_ecp_sync),
	DEVMETHOD(ppbus_read,		ppc_read),
#ifdef DEV_ISA
	DEVMETHOD(ppbus_write,		ppc_isa_write),
#else
	DEVMETHOD(ppbus_write,		ppc_write),
#endif

	{ 0, 0 }
};

static driver_t ppc_acpi_driver = {
	ppc_driver_name,
	ppc_acpi_methods,
	sizeof(struct ppc_data),
};

static struct isa_pnp_id lpc_ids[] = {
	{ 0x0004d041, "Standard parallel printer port" },	/* PNP0400 */
	{ 0x0104d041, "ECP parallel printer port" },		/* PNP0401 */
	{ 0 }
};

static int
ppc_acpi_probe(device_t dev)
{
	device_t parent;
	int error;

	parent = device_get_parent(dev);

	error = ISA_PNP_PROBE(parent, dev, lpc_ids);
	if (error)
		return (ENXIO);

	device_set_desc(dev, "Parallel port");
	return (ppc_probe(dev, 0));
}

DRIVER_MODULE(ppc, acpi, ppc_acpi_driver, ppc_devclass, 0, 0);