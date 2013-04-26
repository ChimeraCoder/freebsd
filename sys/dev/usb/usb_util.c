
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

#ifdef USB_GLOBAL_INCLUDE_FILE
#include USB_GLOBAL_INCLUDE_FILE
#else
#include <sys/stdint.h>
#include <sys/stddef.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/condvar.h>
#include <sys/sysctl.h>
#include <sys/sx.h>
#include <sys/unistd.h>
#include <sys/callout.h>
#include <sys/malloc.h>
#include <sys/priv.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>
#include <dev/usb/usbdi_util.h>

#include <dev/usb/usb_core.h>
#include <dev/usb/usb_util.h>
#include <dev/usb/usb_process.h>
#include <dev/usb/usb_device.h>
#include <dev/usb/usb_request.h>
#include <dev/usb/usb_busdma.h>

#include <dev/usb/usb_controller.h>
#include <dev/usb/usb_bus.h>
#endif			/* USB_GLOBAL_INCLUDE_FILE */

/*------------------------------------------------------------------------*
 *	device_set_usb_desc
 *
 * This function can be called at probe or attach to set the USB
 * device supplied textual description for the given device.
 *------------------------------------------------------------------------*/
void
device_set_usb_desc(device_t dev)
{
	struct usb_attach_arg *uaa;
	struct usb_device *udev;
	struct usb_interface *iface;
	char *temp_p;
	usb_error_t err;
	uint8_t do_unlock;

	if (dev == NULL) {
		/* should not happen */
		return;
	}
	uaa = device_get_ivars(dev);
	if (uaa == NULL) {
		/* can happen if called at the wrong time */
		return;
	}
	udev = uaa->device;
	iface = uaa->iface;

	if ((iface == NULL) ||
	    (iface->idesc == NULL) ||
	    (iface->idesc->iInterface == 0)) {
		err = USB_ERR_INVAL;
	} else {
		err = 0;
	}

	/* Protect scratch area */
	do_unlock = usbd_enum_lock(udev);

	temp_p = (char *)udev->scratch.data;

	if (err == 0) {
		/* try to get the interface string ! */
		err = usbd_req_get_string_any(udev, NULL, temp_p,
		    sizeof(udev->scratch.data),
		    iface->idesc->iInterface);
	}
	if (err != 0) {
		/* use default description */
		usb_devinfo(udev, temp_p,
		    sizeof(udev->scratch.data));
	}

	if (do_unlock)
		usbd_enum_unlock(udev);

	device_set_desc_copy(dev, temp_p);
	device_printf(dev, "<%s> on %s\n", temp_p,
	    device_get_nameunit(udev->bus->bdev));
}

/*------------------------------------------------------------------------*
 *	 usb_pause_mtx - factored out code
 *
 * This function will delay the code by the passed number of system
 * ticks. The passed mutex "mtx" will be dropped while waiting, if
 * "mtx" is different from NULL.
 *------------------------------------------------------------------------*/
void
usb_pause_mtx(struct mtx *mtx, int timo)
{
	if (mtx != NULL)
		mtx_unlock(mtx);

	/*
	 * Add one tick to the timeout so that we don't return too
	 * early! Note that pause() will assert that the passed
	 * timeout is positive and non-zero!
	 */
	pause("USBWAIT", timo + 1);

	if (mtx != NULL)
		mtx_lock(mtx);
}

/*------------------------------------------------------------------------*
 *	usb_printbcd
 *
 * This function will print the version number "bcd" to the string
 * pointed to by "p" having a maximum length of "p_len" bytes
 * including the terminating zero.
 *------------------------------------------------------------------------*/
void
usb_printbcd(char *p, uint16_t p_len, uint16_t bcd)
{
	if (snprintf(p, p_len, "%x.%02x", bcd >> 8, bcd & 0xff)) {
		/* ignore any errors */
	}
}

/*------------------------------------------------------------------------*
 *	usb_trim_spaces
 *
 * This function removes spaces at the beginning and the end of the string
 * pointed to by the "p" argument.
 *------------------------------------------------------------------------*/
void
usb_trim_spaces(char *p)
{
	char *q;
	char *e;

	if (p == NULL)
		return;
	q = e = p;
	while (*q == ' ')		/* skip leading spaces */
		q++;
	while ((*p = *q++))		/* copy string */
		if (*p++ != ' ')	/* remember last non-space */
			e = p;
	*e = 0;				/* kill trailing spaces */
}

/*------------------------------------------------------------------------*
 *	usb_make_str_desc - convert an ASCII string into a UNICODE string
 *------------------------------------------------------------------------*/
uint8_t
usb_make_str_desc(void *ptr, uint16_t max_len, const char *s)
{
	struct usb_string_descriptor *p = ptr;
	uint8_t totlen;
	int j;

	if (max_len < 2) {
		/* invalid length */
		return (0);
	}
	max_len = ((max_len / 2) - 1);

	j = strlen(s);

	if (j < 0) {
		j = 0;
	}
	if (j > 126) {
		j = 126;
	}
	if (max_len > j) {
		max_len = j;
	}
	totlen = (max_len + 1) * 2;

	p->bLength = totlen;
	p->bDescriptorType = UDESC_STRING;

	while (max_len--) {
		USETW2(p->bString[max_len], 0, s[max_len]);
	}
	return (totlen);
}