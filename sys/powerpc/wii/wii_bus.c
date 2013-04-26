
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
#include <sys/module.h>
#include <sys/malloc.h>
#include <sys/bus.h>
#include <sys/clock.h>
#include <sys/cpu.h>
#include <sys/resource.h>
#include <sys/rman.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/bus.h>
#include <machine/platform.h>
#include <machine/pmap.h>
#include <machine/resource.h>

#include <powerpc/wii/wii_picreg.h>
#include <powerpc/wii/wii_fbreg.h>
#include <powerpc/wii/wii_exireg.h>
#include <powerpc/wii/wii_ipcreg.h>
#include <powerpc/wii/wii_gpioreg.h>

static void	wiibus_identify(driver_t *, device_t);
static int	wiibus_probe(device_t);
static int	wiibus_attach(device_t);
static int	wiibus_print_child(device_t, device_t);
static struct resource *
		wiibus_alloc_resource(device_t, device_t, int, int *,
		    unsigned long, unsigned long, unsigned long,
		    unsigned int);
static int	wiibus_activate_resource(device_t, device_t, int, int,
		    struct resource *);

static device_method_t wiibus_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify,	wiibus_identify),
	DEVMETHOD(device_probe,		wiibus_probe),
	DEVMETHOD(device_attach,	wiibus_attach),

        /* Bus interface */
	DEVMETHOD(bus_add_child,	bus_generic_add_child),
	DEVMETHOD(bus_print_child,	wiibus_print_child),
	DEVMETHOD(bus_setup_intr,	bus_generic_setup_intr),
	DEVMETHOD(bus_teardown_intr,	bus_generic_teardown_intr),
	DEVMETHOD(bus_alloc_resource,   wiibus_alloc_resource),
	DEVMETHOD(bus_activate_resource,wiibus_activate_resource),

	DEVMETHOD_END
};

struct wiibus_softc {
	device_t		sc_dev;
	struct rman		sc_rman;
};

static MALLOC_DEFINE(M_WIIBUS, "wiibus", "Nintendo Wii system bus");

struct wiibus_devinfo {
	struct resource_list	di_resources;
	uint8_t			di_init;
};

static driver_t wiibus_driver = {
	"wiibus",
	wiibus_methods,
	sizeof(struct wiibus_softc)
};

static devclass_t wiibus_devclass;

DRIVER_MODULE(wiibus, nexus, wiibus_driver, wiibus_devclass, 0, 0);

static void
wiibus_identify(driver_t *driver, device_t parent)
{

	if (strcmp(installed_platform(), "wii") != 0)
		return;

	if (device_find_child(parent, "wiibus", -1) == NULL)
		BUS_ADD_CHILD(parent, 0, "wiibus", 0);
}


static int
wiibus_probe(device_t dev)
{

	device_set_desc(dev, "Nintendo Wii System Bus");

	return (BUS_PROBE_NOWILDCARD);
}

static void
wiibus_init_device_resources(struct rman *rm, struct wiibus_devinfo *dinfo,
    unsigned int rid, uintptr_t addr, size_t len, unsigned int irq)

{

	if (!dinfo->di_init) {
		resource_list_init(&dinfo->di_resources);
		dinfo->di_init++;
	}
	if (addr) {
		rman_manage_region(rm, addr, addr + len - 1);
		resource_list_add(&dinfo->di_resources, SYS_RES_MEMORY, rid,
		    addr, addr + len, len);
	}
	if (irq)
		resource_list_add(&dinfo->di_resources, SYS_RES_IRQ, rid,
		    irq, irq, 1);
}

static int
wiibus_attach(device_t self)
{
	struct wiibus_softc *sc;
	struct wiibus_devinfo *dinfo;
	device_t cdev;

	sc = device_get_softc(self);
	sc->sc_rman.rm_type = RMAN_ARRAY;
	sc->sc_rman.rm_descr = "Wii Bus Memory Mapped I/O";
	rman_init(&sc->sc_rman);

	/* Nintendo PIC */
	dinfo = malloc(sizeof(*dinfo), M_WIIBUS, M_WAITOK | M_ZERO);
	wiibus_init_device_resources(&sc->sc_rman, dinfo, 0, WIIPIC_REG_ADDR,
	    WIIPIC_REG_LEN, 1);
	cdev = BUS_ADD_CHILD(self, 0, "wiipic", 0);
	device_set_ivars(cdev, dinfo);

	/* Framebuffer */
	dinfo = malloc(sizeof(*dinfo), M_WIIBUS, M_WAITOK | M_ZERO);
	wiibus_init_device_resources(&sc->sc_rman, dinfo, 0, WIIFB_REG_ADDR,
	    WIIFB_REG_LEN, 8);
	wiibus_init_device_resources(&sc->sc_rman, dinfo, 1, WIIFB_FB_ADDR,
	    WIIFB_FB_LEN, 0);
	cdev = BUS_ADD_CHILD(self, 0, "wiifb", 0);
	device_set_ivars(cdev, dinfo);

	/* External Interface Bus */
	dinfo = malloc(sizeof(*dinfo), M_WIIBUS, M_WAITOK | M_ZERO);
	wiibus_init_device_resources(&sc->sc_rman, dinfo, 0, WIIEXI_REG_ADDR,
	    WIIEXI_REG_LEN, 4);
	cdev = BUS_ADD_CHILD(self, 0, "wiiexi", 0);
	device_set_ivars(cdev, dinfo);

	/* Nintendo IOS IPC */
	dinfo = malloc(sizeof(*dinfo), M_WIIBUS, M_WAITOK | M_ZERO);
	wiibus_init_device_resources(&sc->sc_rman, dinfo, 0, WIIIPC_REG_ADDR,
	    WIIIPC_REG_LEN, 14);
	wiibus_init_device_resources(&sc->sc_rman, dinfo, 1, WIIIPC_IOH_ADDR,
	    WIIIPC_IOH_LEN, 0);
	cdev = BUS_ADD_CHILD(self, 0, "wiiipc", 0);
	device_set_ivars(cdev, dinfo);

	/* GPIO */
	dinfo = malloc(sizeof(*dinfo), M_WIIBUS, M_WAITOK | M_ZERO);
	wiibus_init_device_resources(&sc->sc_rman, dinfo, 0, WIIGPIO_REG_ADDR,
	    WIIGPIO_REG_LEN, 0);
	cdev = BUS_ADD_CHILD(self, 0, "wiigpio", 0);
	device_set_ivars(cdev, dinfo);

	return (bus_generic_attach(self));
}

static int
wiibus_print_child(device_t dev, device_t child)
{
	struct wiibus_devinfo *dinfo = device_get_ivars(child);
	int retval = 0;

	retval += bus_print_child_header(dev, child);
	retval += resource_list_print_type(&dinfo->di_resources, "mem",
	    SYS_RES_MEMORY, "%#lx");
	retval += resource_list_print_type(&dinfo->di_resources, "irq",
	    SYS_RES_IRQ, "%ld");
	retval += bus_print_child_footer(dev, child);

	return (retval);
}

static struct resource *
wiibus_alloc_resource(device_t bus, device_t child, int type,
    int *rid, unsigned long start, unsigned long end,
    unsigned long count, unsigned int flags)
{
	struct wiibus_softc *sc;
	struct wiibus_devinfo *dinfo;
	struct resource_list_entry *rle;
	struct resource *rv;
	int needactivate;

	sc = device_get_softc(bus);
	dinfo = device_get_ivars(child);
	needactivate = flags & RF_ACTIVE;
	flags &= ~RF_ACTIVE;

	switch (type) {
	case SYS_RES_MEMORY:
		rle = resource_list_find(&dinfo->di_resources, SYS_RES_MEMORY,
		    *rid);
		if (rle == NULL) {
			device_printf(bus, "no res entry for %s memory 0x%x\n",
			    device_get_nameunit(child), *rid);
			return (NULL);
		}
		rv = rman_reserve_resource(&sc->sc_rman, rle->start, rle->end,
		    rle->count, flags, child);
		if (rv == NULL) {
			device_printf(bus,
			    "failed to reserve resource for %s\n",
			    device_get_nameunit(child));
			return (NULL);
		}
		rman_set_rid(rv, *rid);
		break;
	case SYS_RES_IRQ:
		return (resource_list_alloc(&dinfo->di_resources, bus, child,
		    type, rid, start, end, count, flags));
	default:
		device_printf(bus, "unknown resource request from %s\n",
		    device_get_nameunit(child));
		return (NULL);
	}

	if (needactivate) {
		if (bus_activate_resource(child, type, *rid, rv) != 0) {
			device_printf(bus,
			    "failed to activate resource for %s\n",
			    device_get_nameunit(child));
			return (NULL);
		}
	}
	
	return (rv);
}

static int
wiibus_activate_resource(device_t bus, device_t child, int type, int rid,
    struct resource *res)
{
	void *p;

	switch (type) {
	case SYS_RES_MEMORY:
		p = pmap_mapdev(rman_get_start(res), rman_get_size(res));
		if (p == NULL)
			return (ENOMEM);
		rman_set_virtual(res, p);
		rman_set_bustag(res, &bs_be_tag);
		rman_set_bushandle(res, (unsigned long)p);
		break;
	case SYS_RES_IRQ:
		return (bus_activate_resource(bus, type, rid, res));
	default:
		device_printf(bus,
		    "unknown activate resource request from %s\n",
		    device_get_nameunit(child));
		return (ENXIO);
	}
	
	return (rman_activate_resource(res));
}