
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

#include <dev/usb/usb_core.h>
#include <dev/usb/usb_process.h>
#include <dev/usb/usb_device.h>
#include <dev/usb/usb_dynamic.h>
#endif			/* USB_GLOBAL_INCLUDE_FILE */

/* function prototypes */
static usb_handle_req_t usb_temp_get_desc_w;
static usb_temp_setup_by_index_t usb_temp_setup_by_index_w;
static usb_temp_unsetup_t usb_temp_unsetup_w;
static usb_test_quirk_t usb_test_quirk_w;
static usb_quirk_ioctl_t usb_quirk_ioctl_w;

/* global variables */
usb_handle_req_t *usb_temp_get_desc_p = &usb_temp_get_desc_w;
usb_temp_setup_by_index_t *usb_temp_setup_by_index_p = &usb_temp_setup_by_index_w;
usb_temp_unsetup_t *usb_temp_unsetup_p = &usb_temp_unsetup_w;
usb_test_quirk_t *usb_test_quirk_p = &usb_test_quirk_w;
usb_quirk_ioctl_t *usb_quirk_ioctl_p = &usb_quirk_ioctl_w;
devclass_t usb_devclass_ptr;

static usb_error_t
usb_temp_setup_by_index_w(struct usb_device *udev, uint16_t index)
{
	return (USB_ERR_INVAL);
}

static uint8_t
usb_test_quirk_w(const struct usbd_lookup_info *info, uint16_t quirk)
{
	return (0);			/* no match */
}

static int
usb_quirk_ioctl_w(unsigned long cmd, caddr_t data, int fflag, struct thread *td)
{
	return (ENOIOCTL);
}

static usb_error_t
usb_temp_get_desc_w(struct usb_device *udev, struct usb_device_request *req, const void **pPtr, uint16_t *pLength)
{
	/* stall */
	return (USB_ERR_STALLED);
}

static void
usb_temp_unsetup_w(struct usb_device *udev)
{
	if (udev->usb_template_ptr) {

		free(udev->usb_template_ptr, M_USB);

		udev->usb_template_ptr = NULL;
	}
}

void
usb_quirk_unload(void *arg)
{
	/* reset function pointers */

	usb_test_quirk_p = &usb_test_quirk_w;
	usb_quirk_ioctl_p = &usb_quirk_ioctl_w;

	/* wait for CPU to exit the loaded functions, if any */

	/* XXX this is a tradeoff */

	pause("WAIT", hz);
}

void
usb_temp_unload(void *arg)
{
	/* reset function pointers */

	usb_temp_get_desc_p = &usb_temp_get_desc_w;
	usb_temp_setup_by_index_p = &usb_temp_setup_by_index_w;
	usb_temp_unsetup_p = &usb_temp_unsetup_w;

	/* wait for CPU to exit the loaded functions, if any */

	/* XXX this is a tradeoff */

	pause("WAIT", hz);
}

void
usb_bus_unload(void *arg)
{
	/* reset function pointers */

	usb_devclass_ptr = NULL;

	/* wait for CPU to exit the loaded functions, if any */

	/* XXX this is a tradeoff */

	pause("WAIT", hz);
}