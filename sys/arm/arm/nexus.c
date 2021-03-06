
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
 * This code implements a `root nexus' for Arm Architecture
 * machines.  The function of the root nexus is to serve as an
 * attachment point for both processors and buses, and to manage
 * resources which are common to all of them.  In particular,
 * this code implements the core resource managers for interrupt
 * requests, DMA requests (which rightfully should be a part of the
 * ISA code but it's easier to do it here for now), I/O port addresses,
 * and I/O memory address space.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <sys/interrupt.h>

#include <machine/vmparam.h>
#include <machine/pcb.h>
#include <vm/vm.h>
#include <vm/pmap.h>
#include <machine/pmap.h>

#include <machine/resource.h>
#include <machine/intr.h>

static MALLOC_DEFINE(M_NEXUSDEV, "nexusdev", "Nexus device");

struct nexus_device {
	struct resource_list	nx_resources;
};

#define DEVTONX(dev)	((struct nexus_device *)device_get_ivars(dev))

static struct rman mem_rman;

static	int nexus_probe(device_t);
static	int nexus_attach(device_t);
static	int nexus_print_child(device_t, device_t);
static	device_t nexus_add_child(device_t, u_int, const char *, int);
static	struct resource *nexus_alloc_resource(device_t, device_t, int, int *,
    u_long, u_long, u_long, u_int);
static	int nexus_activate_resource(device_t, device_t, int, int,
    struct resource *);
static int nexus_setup_intr(device_t dev, device_t child, struct resource *res,
    int flags, driver_filter_t *filt, driver_intr_t *intr, void *arg, void **cookiep);
static int nexus_teardown_intr(device_t, device_t, struct resource *, void *);

static device_method_t nexus_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		nexus_probe),
	DEVMETHOD(device_attach,	nexus_attach),
	/* Bus interface */
	DEVMETHOD(bus_print_child,	nexus_print_child),
	DEVMETHOD(bus_add_child,	nexus_add_child),
	DEVMETHOD(bus_alloc_resource,	nexus_alloc_resource),
	DEVMETHOD(bus_activate_resource,	nexus_activate_resource),
	DEVMETHOD(bus_setup_intr,	nexus_setup_intr),
	DEVMETHOD(bus_teardown_intr,	nexus_teardown_intr),
	{ 0, 0 }
};

static driver_t nexus_driver = {
	"nexus",
	nexus_methods,
	1			/* no softc */
};
static devclass_t nexus_devclass;

static int
nexus_probe(device_t dev)
{

	device_quiet(dev);	/* suppress attach message for neatness */

	return (BUS_PROBE_DEFAULT);
}

static int
nexus_setup_intr(device_t dev, device_t child, struct resource *res, int flags,
    driver_filter_t *filt, driver_intr_t *intr, void *arg, void **cookiep)
{
	int irq;

	if ((rman_get_flags(res) & RF_SHAREABLE) == 0)
		flags |= INTR_EXCL;

	for (irq = rman_get_start(res); irq <= rman_get_end(res); irq++) {
		arm_setup_irqhandler(device_get_nameunit(child),
		    filt, intr, arg, irq, flags, cookiep);
		arm_unmask_irq(irq);
	}
	return (0);
}

static int
nexus_teardown_intr(device_t dev, device_t child, struct resource *r, void *ih)
{

	return (arm_remove_irqhandler(rman_get_start(r), ih));
}

static int
nexus_attach(device_t dev)
{

	mem_rman.rm_start = 0;
	mem_rman.rm_end = ~0ul;
	mem_rman.rm_type = RMAN_ARRAY;
	mem_rman.rm_descr = "I/O memory addresses";
	if (rman_init(&mem_rman) || rman_manage_region(&mem_rman, 0, ~0))
		panic("nexus_probe mem_rman");

	/*
	 * First, deal with the children we know about already
	 */
	bus_generic_probe(dev);
	bus_generic_attach(dev);

	return (0);
}


static int
nexus_print_child(device_t bus, device_t child)
{
	int retval = 0;

	retval += bus_print_child_header(bus, child);
	retval += printf("\n");

	return (retval);
}

static device_t
nexus_add_child(device_t bus, u_int order, const char *name, int unit)
{
	device_t child;
	struct nexus_device *ndev;

	ndev = malloc(sizeof(struct nexus_device), M_NEXUSDEV, M_NOWAIT|M_ZERO);
	if (!ndev)
		return (0);
	resource_list_init(&ndev->nx_resources);

	child = device_add_child_ordered(bus, order, name, unit);

	/* should we free this in nexus_child_detached? */
	device_set_ivars(child, ndev);

	return (child);
}


/*
 * Allocate a resource on behalf of child.  NB: child is usually going to be a
 * child of one of our descendants, not a direct child of nexus0.
 * (Exceptions include footbridge.)
 */
#define ARM_BUS_SPACE_MEM 1
static struct resource *
nexus_alloc_resource(device_t bus, device_t child, int type, int *rid,
    u_long start, u_long end, u_long count, u_int flags)
{
	struct resource *rv;
	struct rman *rm;
	int needactivate = flags & RF_ACTIVE;

	switch (type) {
	case SYS_RES_MEMORY:
		rm = &mem_rman;
		break;

	default:
		return (0);
	}

	rv = rman_reserve_resource(rm, start, end, count, flags, child);
	if (rv == 0)
		return (0);

	rman_set_rid(rv, *rid);
	rman_set_bustag(rv, (void*)ARM_BUS_SPACE_MEM);
	rman_set_bushandle(rv, rman_get_start(rv));

	if (needactivate) {
		if (bus_activate_resource(child, type, *rid, rv)) {
			rman_release_resource(rv);
			return (0);
		}
	}

	return (rv);
}


static int
nexus_activate_resource(device_t bus, device_t child, int type, int rid,
    struct resource *r)
{
	/*
	 * If this is a memory resource, map it into the kernel.
	 */
	if (rman_get_bustag(r) == (void*)ARM_BUS_SPACE_MEM) {
		caddr_t vaddr = 0;
		u_int32_t paddr;
		u_int32_t psize;
		u_int32_t poffs;

		paddr = rman_get_start(r);
		psize = rman_get_size(r);
		poffs = paddr - trunc_page(paddr);
		vaddr = (caddr_t) pmap_mapdev(paddr-poffs, psize+poffs) + poffs;
		rman_set_virtual(r, vaddr);
		rman_set_bushandle(r, (bus_space_handle_t) vaddr);
	}
	return (rman_activate_resource(r));
}

DRIVER_MODULE(nexus, root, nexus_driver, nexus_devclass, 0, 0);