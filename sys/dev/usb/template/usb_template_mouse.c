
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
 * This file contains the USB template for an USB Mouse Device.
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
#include <dev/usb/usb_cdc.h>

#include <dev/usb/template/usb_template.h>
#endif			/* USB_GLOBAL_INCLUDE_FILE */

enum {
	INDEX_LANG,
	INDEX_MOUSE,
	INDEX_PRODUCT,
	INDEX_MAX,
};

#define	STRING_PRODUCT \
  "M\0o\0u\0s\0e\0 \0T\0e\0s\0t\0 \0D\0e\0v\0i\0c\0e"

#define	STRING_MOUSE \
  "M\0o\0u\0s\0e\0 \0i\0n\0t\0e\0r\0f\0a\0c\0e"

/* make the real string descriptors */

USB_MAKE_STRING_DESC(STRING_MOUSE, string_mouse);
USB_MAKE_STRING_DESC(STRING_PRODUCT, string_product);

/* prototypes */

/* The following HID descriptor was dumped from a HP mouse. */

static uint8_t mouse_hid_descriptor[] = {
	0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x09, 0x01,
	0xa1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x03,
	0x15, 0x00, 0x25, 0x01, 0x95, 0x03, 0x75, 0x01,
	0x81, 0x02, 0x95, 0x05, 0x81, 0x03, 0x05, 0x01,
	0x09, 0x30, 0x09, 0x31, 0x09, 0x38, 0x15, 0x81,
	0x25, 0x7f, 0x75, 0x08, 0x95, 0x03, 0x81, 0x06,
	0xc0, 0xc0
};

static const struct usb_temp_packet_size mouse_intr_mps = {
	.mps[USB_SPEED_LOW] = 8,
	.mps[USB_SPEED_FULL] = 8,
	.mps[USB_SPEED_HIGH] = 8,
};

static const struct usb_temp_interval mouse_intr_interval = {
	.bInterval[USB_SPEED_LOW] = 2,		/* 2ms */
	.bInterval[USB_SPEED_FULL] = 2,		/* 2ms */
	.bInterval[USB_SPEED_HIGH] = 5,		/* 2ms */
};

static const struct usb_temp_endpoint_desc mouse_ep_0 = {
	.ppRawDesc = NULL,		/* no raw descriptors */
	.pPacketSize = &mouse_intr_mps,
	.pIntervals = &mouse_intr_interval,
	.bEndpointAddress = UE_DIR_IN,
	.bmAttributes = UE_INTERRUPT,
};

static const struct usb_temp_endpoint_desc *mouse_endpoints[] = {
	&mouse_ep_0,
	NULL,
};

static const uint8_t mouse_raw_desc[] = {
	0x09, 0x21, 0x10, 0x01, 0x00, 0x01, 0x22, sizeof(mouse_hid_descriptor),
	0x00
};

static const void *mouse_iface_0_desc[] = {
	mouse_raw_desc,
	NULL,
};

static const struct usb_temp_interface_desc mouse_iface_0 = {
	.ppRawDesc = mouse_iface_0_desc,
	.ppEndpoints = mouse_endpoints,
	.bInterfaceClass = 3,
	.bInterfaceSubClass = 1,
	.bInterfaceProtocol = 2,
	.iInterface = INDEX_MOUSE,
};

static const struct usb_temp_interface_desc *mouse_interfaces[] = {
	&mouse_iface_0,
	NULL,
};

static const struct usb_temp_config_desc mouse_config_desc = {
	.ppIfaceDesc = mouse_interfaces,
	.bmAttributes = UC_BUS_POWERED,
	.bMaxPower = 25,		/* 50 mA */
	.iConfiguration = INDEX_PRODUCT,
};

static const struct usb_temp_config_desc *mouse_configs[] = {
	&mouse_config_desc,
	NULL,
};

static usb_temp_get_string_desc_t mouse_get_string_desc;
static usb_temp_get_vendor_desc_t mouse_get_vendor_desc;

const struct usb_temp_device_desc usb_template_mouse = {
	.getStringDesc = &mouse_get_string_desc,
	.getVendorDesc = &mouse_get_vendor_desc,
	.ppConfigDesc = mouse_configs,
	.idVendor = USB_TEMPLATE_VENDOR,
	.idProduct = 0x00AE,
	.bcdDevice = 0x0100,
	.bDeviceClass = UDCLASS_COMM,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.iManufacturer = 0,
	.iProduct = INDEX_PRODUCT,
	.iSerialNumber = 0,
};

/*------------------------------------------------------------------------*
 *      mouse_get_vendor_desc
 *
 * Return values:
 * NULL: Failure. No such vendor descriptor.
 * Else: Success. Pointer to vendor descriptor is returned.
 *------------------------------------------------------------------------*/
static const void *
mouse_get_vendor_desc(const struct usb_device_request *req, uint16_t *plen)
{
	if ((req->bmRequestType == 0x81) && (req->bRequest == 0x06) &&
	    (req->wValue[0] == 0x00) && (req->wValue[1] == 0x22) &&
	    (req->wIndex[1] == 0) && (req->wIndex[0] == 0)) {

		*plen = sizeof(mouse_hid_descriptor);
		return (mouse_hid_descriptor);
	}
	return (NULL);
}

/*------------------------------------------------------------------------*
 *	mouse_get_string_desc
 *
 * Return values:
 * NULL: Failure. No such string.
 * Else: Success. Pointer to string descriptor is returned.
 *------------------------------------------------------------------------*/
static const void *
mouse_get_string_desc(uint16_t lang_id, uint8_t string_index)
{
	static const void *ptr[INDEX_MAX] = {
		[INDEX_LANG] = &usb_string_lang_en,
		[INDEX_MOUSE] = &string_mouse,
		[INDEX_PRODUCT] = &string_product,
	};

	if (string_index == 0) {
		return (&usb_string_lang_en);
	}
	if (lang_id != 0x0409) {
		return (NULL);
	}
	if (string_index < INDEX_MAX) {
		return (ptr[string_index]);
	}
	return (NULL);
}