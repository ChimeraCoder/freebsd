
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

/**
 * \file xenbusb_front.c
 *
 * XenBus management of the NewBus bus containing the frontend instances of
 * Xen split devices.
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/sbuf.h>
#include <sys/sysctl.h>
#include <sys/syslog.h>
#include <sys/systm.h>
#include <sys/sx.h>
#include <sys/taskqueue.h>

#include <machine/xen/xen-os.h>
#include <machine/stdarg.h>

#include <xen/gnttab.h>
#include <xen/xenbus/xenbusvar.h>
#include <xen/xenbus/xenbusb.h>


/*------------------ Private Device Attachment Functions  --------------------*/
/**
 * \brief Probe for the existance of the XenBus front bus.
 *
 * \param dev  NewBus device_t for this XenBus front bus instance.
 *
 * \return  Always returns 0 indicating success.
 */
static int 
xenbusb_front_probe(device_t dev)
{
	device_set_desc(dev, "Xen Frontend Devices");

	return (0);
}

/**
 * \brief Attach the XenBus front bus.
 *
 * \param dev  NewBus device_t for this XenBus front bus instance.
 *
 * \return  On success, 0. Otherwise an errno value indicating the
 *          type of failure.
 */
static int
xenbusb_front_attach(device_t dev)
{
	return (xenbusb_attach(dev, "device", /*id_components*/1));
}

/**
 * \brief Enumerate all devices of the given type on this bus.
 *
 * \param dev   NewBus device_t for this XenBus front bus instance.
 * \param type  String indicating the device sub-tree (e.g. "vfb", "vif")
 *              to enumerate. 
 *
 * \return  On success, 0. Otherwise an errno value indicating the
 *          type of failure.
 *
 * Devices that are found are entered into the NewBus hierarchy via
 * xenbusb_add_device().  xenbusb_add_device() ignores duplicate detects
 * and ignores duplicate devices, so it can be called unconditionally
 * for any device found in the XenStore.
 */
static int
xenbusb_front_enumerate_type(device_t dev, const char *type)
{
	struct xenbusb_softc *xbs;
	const char **dir;
	unsigned int i, count;
	int error;

	xbs = device_get_softc(dev);
	error = xs_directory(XST_NIL, xbs->xbs_node, type, &count, &dir);
	if (error)
		return (error);
	for (i = 0; i < count; i++)
		xenbusb_add_device(dev, type, dir[i]);

	free(dir, M_XENSTORE);

	return (0);
}

/**
 * \brief Determine and store the XenStore path for the other end of
 *        a split device whose local end is represented by ivars.
 *
 * If successful, the xd_otherend_path field of the child's instance
 * variables will be updated.
 *
 * \param dev    NewBus device_t for this XenBus front bus instance.
 * \param ivars  Instance variables from the XenBus child device for
 *               which to perform this function.
 *
 * \return  On success, 0. Otherwise an errno value indicating the
 *          type of failure.
 */
static int
xenbusb_front_get_otherend_node(device_t dev, struct xenbus_device_ivars *ivars)
{
	char *otherend_path;
	int error;

	if (ivars->xd_otherend_path != NULL) {
		free(ivars->xd_otherend_path, M_XENBUS);
		ivars->xd_otherend_path = NULL;
	}
		
	error = xs_gather(XST_NIL, ivars->xd_node,
	    "backend-id", "%i", &ivars->xd_otherend_id,
	    "backend", NULL, &otherend_path,
	    NULL);

	if (error == 0) {
		ivars->xd_otherend_path = strdup(otherend_path, M_XENBUS);
		ivars->xd_otherend_path_len = strlen(otherend_path);
		free(otherend_path, M_XENSTORE);
	}
	return (error);
}

/*-------------------- Private Device Attachment Data  -----------------------*/
static device_method_t xenbusb_front_methods[] = { 
	/* Device interface */ 
	DEVMETHOD(device_identify,	xenbusb_identify),
	DEVMETHOD(device_probe,         xenbusb_front_probe), 
	DEVMETHOD(device_attach,        xenbusb_front_attach), 
	DEVMETHOD(device_detach,        bus_generic_detach), 
	DEVMETHOD(device_shutdown,      bus_generic_shutdown), 
	DEVMETHOD(device_suspend,       bus_generic_suspend), 
	DEVMETHOD(device_resume,        xenbusb_resume), 
 
	/* Bus Interface */ 
	DEVMETHOD(bus_print_child,      xenbusb_print_child),
	DEVMETHOD(bus_read_ivar,        xenbusb_read_ivar), 
	DEVMETHOD(bus_write_ivar,       xenbusb_write_ivar), 
	DEVMETHOD(bus_alloc_resource,   bus_generic_alloc_resource),
	DEVMETHOD(bus_release_resource, bus_generic_release_resource),
	DEVMETHOD(bus_activate_resource, bus_generic_activate_resource),
	DEVMETHOD(bus_deactivate_resource, bus_generic_deactivate_resource),
 
	/* XenBus Bus Interface */
	DEVMETHOD(xenbusb_enumerate_type, xenbusb_front_enumerate_type),
	DEVMETHOD(xenbusb_get_otherend_node, xenbusb_front_get_otherend_node),
	{ 0, 0 } 
}; 

DEFINE_CLASS_0(xenbusb_front, xenbusb_front_driver, xenbusb_front_methods,
	       sizeof(struct xenbusb_softc));
devclass_t xenbusb_front_devclass; 
 
DRIVER_MODULE(xenbusb_front, xenstore, xenbusb_front_driver,
	      xenbusb_front_devclass, 0, 0);