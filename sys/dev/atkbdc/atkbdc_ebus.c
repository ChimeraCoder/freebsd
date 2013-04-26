
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

#include "opt_kbd.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/kbio.h>
#include <sys/malloc.h>

#include <dev/ofw/ofw_bus.h>

#include <machine/resource.h>
#include <machine/ver.h>

#include <sys/rman.h>

#include <dev/kbd/kbdreg.h>
#include <dev/atkbdc/atkbdreg.h>
#include <dev/atkbdc/atkbdc_subr.h>
#include <dev/atkbdc/atkbdcreg.h>
#include <dev/atkbdc/psm.h>

static device_probe_t atkbdc_ebus_probe;
static device_attach_t atkbdc_ebus_attach;

static device_method_t atkbdc_ebus_methods[] = {
	DEVMETHOD(device_probe,		atkbdc_ebus_probe),
	DEVMETHOD(device_attach,	atkbdc_ebus_attach),
	DEVMETHOD(device_suspend,	bus_generic_suspend),
	DEVMETHOD(device_resume,	bus_generic_resume),

	DEVMETHOD(bus_print_child,	atkbdc_print_child),
	DEVMETHOD(bus_read_ivar,	atkbdc_read_ivar),
	DEVMETHOD(bus_write_ivar,	atkbdc_write_ivar),
	DEVMETHOD(bus_get_resource_list,atkbdc_get_resource_list),
	DEVMETHOD(bus_alloc_resource,	bus_generic_rl_alloc_resource),
	DEVMETHOD(bus_release_resource,	bus_generic_rl_release_resource),
	DEVMETHOD(bus_activate_resource, bus_generic_activate_resource),
	DEVMETHOD(bus_deactivate_resource, bus_generic_deactivate_resource),
	DEVMETHOD(bus_get_resource,	bus_generic_rl_get_resource),
	DEVMETHOD(bus_set_resource,	bus_generic_rl_set_resource),
	DEVMETHOD(bus_delete_resource,	bus_generic_rl_delete_resource),
	DEVMETHOD(bus_setup_intr,	bus_generic_setup_intr),
	DEVMETHOD(bus_teardown_intr,	bus_generic_teardown_intr),

	{ 0, 0 }
};

static driver_t atkbdc_ebus_driver = {
	ATKBDC_DRIVER_NAME,
	atkbdc_ebus_methods,
	sizeof(atkbdc_softc_t *),
};

DRIVER_MODULE(atkbdc, ebus, atkbdc_ebus_driver, atkbdc_devclass, 0, 0);

static int
atkbdc_ebus_probe(device_t dev)
{
	struct resource *port0, *port1;
	u_long count, start;
	int error, rid;

	if (strcmp(ofw_bus_get_name(dev), "8042") != 0)
		return (ENXIO);

	/*
	 * On AXi and AXmp boards the NS16550 (used to connect keyboard/
	 * mouse) share their IRQ lines with the i8042. Any IRQ activity
	 * (typically during attach) of the NS16550 used to connect the
	 * keyboard when actually the PS/2 keyboard is selected in OFW
	 * causes interaction with the OBP i8042 driver resulting in a
	 * hang and vice versa. As RS232 keyboards and mice obviously
	 * aren't meant to be used in parallel with PS/2 ones on these
	 * boards don't attach to the i8042 in case the PS/2 keyboard
	 * isn't selected in order to prevent such hangs.
	 * Note that it's not sufficient here to rely on the '8042' node
	 * only showing up when a PS/2 keyboard is actually connected as
	 * the user still might have adjusted the 'keyboard' alias to
	 * point to the RS232 keyboard.
	 */
	if ((!strcmp(sparc64_model, "SUNW,UltraAX-MP") ||
	    !strcmp(sparc64_model, "SUNW,UltraSPARC-IIi-Engine")) &&
	    OF_finddevice("keyboard") != ofw_bus_get_node(dev)) {
		device_disable(dev);
		return (ENXIO);
	}

	device_set_desc(dev, "Keyboard controller (i8042)");

	/*
	 * The '8042' node has two identical 8 addresses wide resources
	 * which are apparently meant to be used one for the keyboard
	 * half and the other one for the mouse half. To simplify matters
	 * we use one for the command/data port resource and the other
	 * one for the status port resource as the atkbdc(4) back-end
	 * expects two struct resource rather than two bus space handles.
	 */
	rid = 0;
	if (bus_get_resource(dev, SYS_RES_MEMORY, rid, &start, &count) != 0) {
		device_printf(dev,
		    "cannot determine command/data port resource\n");
		return (ENXIO);
	}
	port0 = bus_alloc_resource(dev, SYS_RES_MEMORY, &rid, start, start, 1,
	    RF_ACTIVE);
	if (port0 == NULL) {
		device_printf(dev,
		    "cannot allocate command/data port resource\n");
		return (ENXIO);
	}

	rid = 1;
	if (bus_get_resource(dev, SYS_RES_MEMORY, rid, &start, &count) != 0) {
		device_printf(dev, "cannot determine status port resource\n");
		error = ENXIO;
		goto fail_port0;
	}
	start += KBD_STATUS_PORT;
	port1 = bus_alloc_resource(dev, SYS_RES_MEMORY, &rid, start, start, 1,
	    RF_ACTIVE);
	if (port1 == NULL) {
		device_printf(dev, "cannot allocate status port resource\n");
		error = ENXIO;
		goto fail_port0;
	}

	error = atkbdc_probe_unit(device_get_unit(dev), port0, port1);
	if (error != 0)
		device_printf(dev, "atkbdc_porbe_unit failed\n");

	bus_release_resource(dev, SYS_RES_MEMORY, 1, port1);
 fail_port0:
	bus_release_resource(dev, SYS_RES_MEMORY, 0, port0);

	return (error);
}

static int
atkbdc_ebus_attach(device_t dev)
{
	atkbdc_softc_t *sc;
	atkbdc_device_t *adi;
	device_t cdev;
	phandle_t child;
	u_long count, intr, start;
	int children, error, rid, unit;
	char *cname, *dname;

	unit = device_get_unit(dev);
	sc = *(atkbdc_softc_t **)device_get_softc(dev);
	if (sc == NULL) {
		/*
		 * We have to maintain two copies of the kbdc_softc struct,
		 * as the low-level console needs to have access to the
		 * keyboard controller before kbdc is probed and attached.
		 * kbdc_soft[] contains the default entry for that purpose.
		 * See atkbdc.c. XXX
		 */
		sc = atkbdc_get_softc(unit);
		if (sc == NULL)
			return (ENOMEM);
		device_set_softc(dev, sc);
	}

	rid = 0;
	if (bus_get_resource(dev, SYS_RES_MEMORY, rid, &start, &count) != 0) {
		device_printf(dev,
		    "cannot determine command/data port resource\n");
		return (ENXIO);
	}
	sc->retry = 5000;
	sc->port0 = bus_alloc_resource(dev, SYS_RES_MEMORY, &rid, start, start,
	    1, RF_ACTIVE);
	if (sc->port0 == NULL) {
		device_printf(dev,
		    "cannot allocate command/data port resource\n");
		return (ENXIO);
	}

	rid = 1;
	if (bus_get_resource(dev, SYS_RES_MEMORY, rid, &start, &count) != 0) {
		device_printf(dev, "cannot determine status port resource\n");
		error = ENXIO;
		goto fail_port0;
	}
	start += KBD_STATUS_PORT;
	sc->port1 = bus_alloc_resource(dev, SYS_RES_MEMORY, &rid, start, start,
	    1, RF_ACTIVE);
	if (sc->port1 == NULL) {
		device_printf(dev, "cannot allocate status port resource\n");
		error = ENXIO;
		goto fail_port0;
	}

	error = atkbdc_attach_unit(unit, sc, sc->port0, sc->port1);
	if (error != 0) {
		device_printf(dev, "atkbdc_attach_unit failed\n");
		goto fail_port1;
	}

	/* Attach children. */
	children = 0;
	for (child = OF_child(ofw_bus_get_node(dev)); child != 0;
	    child = OF_peer(child)) {
		if ((OF_getprop_alloc(child, "name", 1, (void **)&cname)) == -1)
			continue;
		if (children >= 2) {
			device_printf(dev,
			    "<%s>: only two children per 8042 supported\n",
			    cname);
			free(cname, M_OFWPROP);
			continue;
		}
		adi = malloc(sizeof(struct atkbdc_device), M_ATKBDDEV,
		    M_NOWAIT | M_ZERO);
		if (adi == NULL) {
			device_printf(dev, "<%s>: malloc failed\n", cname);
			free(cname, M_OFWPROP);
			continue;
		}
		if (strcmp(cname, "kb_ps2") == 0) {
			adi->rid = KBDC_RID_KBD;
			dname = ATKBD_DRIVER_NAME;
		} else if (strcmp(cname, "kdmouse") == 0) {
			adi->rid = KBDC_RID_AUX;
			dname = PSM_DRIVER_NAME;
		} else {
			device_printf(dev, "<%s>: unknown device\n", cname);
			free(adi, M_ATKBDDEV);
			free(cname, M_OFWPROP);
			continue;
		}
		intr = bus_get_resource_start(dev, SYS_RES_IRQ, adi->rid);
		if (intr == 0) {
			device_printf(dev,
			    "<%s>: cannot determine interrupt resource\n",
			    cname);
			free(adi, M_ATKBDDEV);
			free(cname, M_OFWPROP);
			continue;
		}
		resource_list_init(&adi->resources);
		resource_list_add(&adi->resources, SYS_RES_IRQ, adi->rid,
		    intr, intr, 1);
		if ((cdev = device_add_child(dev, dname, -1)) == NULL) {
			device_printf(dev, "<%s>: device_add_child failed\n",
			    cname);
			resource_list_free(&adi->resources);
			free(adi, M_ATKBDDEV);
			free(cname, M_OFWPROP);
			continue;
		}
		device_set_ivars(cdev, adi);
		children++;
	}

	error = bus_generic_attach(dev);
	if (error != 0) {
		device_printf(dev, "bus_generic_attach failed\n");
		goto fail_port1;
	}

	return (0);

 fail_port1:
	bus_release_resource(dev, SYS_RES_MEMORY, 1, sc->port1);
 fail_port0:
	bus_release_resource(dev, SYS_RES_MEMORY, 0, sc->port0);

	return (error);
}