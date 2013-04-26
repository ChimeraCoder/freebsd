
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
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/proc.h>

#ifdef __FreeBSD__
#include <sys/module.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <machine/md_var.h>

#include <isa/isareg.h>
#include <isa/isavar.h>
#else
#include <sys/device.h>

#include <machine/cpu.h>
#include <machine/bus.h>
#include <machine/intr.h>
#include <machine/conf.h>

#include <dev/isa/isareg.h>
#include <dev/isa/isavar.h>
#endif
#include "tpmvar.h"

static int
tpm_isa_probe(device_t dev)
{
	bus_space_tag_t iot;
	bus_space_handle_t ioh;
	struct resource *mem_res;
	int rv, mem_rid;

	mem_rid = 0;
	mem_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &mem_rid,
	    RF_ACTIVE);
	if (mem_res == NULL)
		return (ENXIO);
	iot = rman_get_bustag(mem_res);
	ioh = rman_get_bushandle(mem_res);

	if ((rv = tpm_tis12_probe(iot, ioh)))
		device_set_desc(dev, "Trusted Platform Module");

	bus_release_resource(dev, SYS_RES_MEMORY, mem_rid, mem_res);
	return rv ? 0 : ENXIO;
}

static device_method_t tpm_methods[] = {
#if 0
	DEVMETHOD(device_identify,	tpm_identify),
#endif
	DEVMETHOD(device_probe,		tpm_isa_probe),
	DEVMETHOD(device_attach,	tpm_attach),
	DEVMETHOD(device_detach,	tpm_detach),
	DEVMETHOD(device_suspend,	tpm_suspend),
	DEVMETHOD(device_resume,	tpm_resume),
	{ 0, 0 }
};

static driver_t tpm_driver = {
	"tpm", tpm_methods, sizeof(struct tpm_softc),
};

static devclass_t tpm_devclass;

DRIVER_MODULE(tpm, isa, tpm_driver, tpm_devclass, 0, 0);