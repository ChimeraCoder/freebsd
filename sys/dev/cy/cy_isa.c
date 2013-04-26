
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
 * Cyclades Y ISA serial interface driver
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <isa/isavar.h>

#include <dev/cy/cyreg.h>
#include <dev/cy/cyvar.h>

static int	cy_isa_attach(device_t dev);
static int	cy_isa_probe(device_t dev);

static device_method_t cy_isa_methods[] = {
	/* Device interface. */
	DEVMETHOD(device_probe,		cy_isa_probe),
	DEVMETHOD(device_attach,	cy_isa_attach),

	{ 0, 0 }
};

static driver_t cy_isa_driver = {
	cy_driver_name,
	cy_isa_methods,
	0,
};

DRIVER_MODULE(cy, isa, cy_isa_driver, cy_devclass, 0, 0);

static int
cy_isa_probe(device_t dev)
{
	struct resource *mem_res;
	cy_addr iobase;
	int error, mem_rid;

	if (isa_get_logicalid(dev) != 0)	/* skip PnP probes */
		return (ENXIO);

	mem_rid = 0;
	mem_res = bus_alloc_resource(dev, SYS_RES_MEMORY, &mem_rid,
	    0ul, ~0ul, 0ul, RF_ACTIVE);
	if (mem_res == NULL) {
		device_printf(dev, "ioport resource allocation failed\n");
		return (ENXIO);
	}
	iobase = rman_get_virtual(mem_res);

	/* Cyclom-16Y hardware reset (Cyclom-8Ys don't care) */
	cy_inb(iobase, CY16_RESET, 0);	/* XXX? */
	DELAY(500);		/* wait for the board to get its act together */

	/* this is needed to get the board out of reset */
	cy_outb(iobase, CY_CLEAR_INTR, 0, 0);
	DELAY(500);

	error = (cy_units(iobase, 0) == 0 ? ENXIO : 0);
	bus_release_resource(dev, SYS_RES_MEMORY, mem_rid, mem_res);
	return (error);
}

static int
cy_isa_attach(device_t dev)
{
	struct resource *irq_res, *mem_res;
	void *irq_cookie, *vaddr, *vsc;
	int irq_rid, mem_rid;

	irq_res = NULL;
	mem_res = NULL;

	mem_rid = 0;
	mem_res = bus_alloc_resource(dev, SYS_RES_MEMORY, &mem_rid,
	    0ul, ~0ul, 0ul, RF_ACTIVE);
	if (mem_res == NULL) {
		device_printf(dev, "memory resource allocation failed\n");
		goto fail;
	}
	vaddr = rman_get_virtual(mem_res);

	vsc = cyattach_common(vaddr, 0);
	if (vsc == NULL) {
		device_printf(dev, "no ports found!\n");
		goto fail;
	}

	irq_rid = 0;
	irq_res = bus_alloc_resource(dev, SYS_RES_IRQ, &irq_rid, 0ul, ~0ul, 0ul,
	    RF_SHAREABLE | RF_ACTIVE);
	if (irq_res == NULL) {
		device_printf(dev, "interrupt resource allocation failed\n");
		goto fail;
	}
	if (bus_setup_intr(dev, irq_res, INTR_TYPE_TTY, 
	    cyintr, NULL, vsc, &irq_cookie) != 0) {	    
		device_printf(dev, "interrupt setup failed\n");
		goto fail;
	}

	return (0);

fail:
	if (irq_res != NULL)
		bus_release_resource(dev, SYS_RES_IRQ, irq_rid, irq_res);
	if (mem_res != NULL)
		bus_release_resource(dev, SYS_RES_MEMORY, mem_rid, mem_res);
	return (ENXIO);
}