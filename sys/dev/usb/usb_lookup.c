
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
#include <sys/limits.h>
#include <sys/endian.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>
#endif			/* USB_GLOBAL_INCLUDE_FILE */

/*------------------------------------------------------------------------*
 *	usbd_lookup_id_by_info
 *
 * This functions takes an array of "struct usb_device_id" and tries
 * to match the entries with the information in "struct usbd_lookup_info".
 *
 * NOTE: The "sizeof_id" parameter must be a multiple of the
 * usb_device_id structure size. Else the behaviour of this function
 * is undefined.
 *
 * Return values:
 * NULL: No match found.
 * Else: Pointer to matching entry.
 *------------------------------------------------------------------------*/
const struct usb_device_id *
usbd_lookup_id_by_info(const struct usb_device_id *id, usb_size_t sizeof_id,
    const struct usbd_lookup_info *info)
{
	const struct usb_device_id *id_end;

	if (id == NULL) {
		goto done;
	}
	id_end = (const void *)(((const uint8_t *)id) + sizeof_id);

	/*
	 * Keep on matching array entries until we find a match or
	 * until we reach the end of the matching array:
	 */
	for (; id != id_end; id++) {

		if ((id->match_flag_vendor) &&
		    (id->idVendor != info->idVendor)) {
			continue;
		}
		if ((id->match_flag_product) &&
		    (id->idProduct != info->idProduct)) {
			continue;
		}
		if ((id->match_flag_dev_lo) &&
		    (id->bcdDevice_lo > info->bcdDevice)) {
			continue;
		}
		if ((id->match_flag_dev_hi) &&
		    (id->bcdDevice_hi < info->bcdDevice)) {
			continue;
		}
		if ((id->match_flag_dev_class) &&
		    (id->bDeviceClass != info->bDeviceClass)) {
			continue;
		}
		if ((id->match_flag_dev_subclass) &&
		    (id->bDeviceSubClass != info->bDeviceSubClass)) {
			continue;
		}
		if ((id->match_flag_dev_protocol) &&
		    (id->bDeviceProtocol != info->bDeviceProtocol)) {
			continue;
		}
		if ((id->match_flag_int_class) &&
		    (id->bInterfaceClass != info->bInterfaceClass)) {
			continue;
		}
		if ((id->match_flag_int_subclass) &&
		    (id->bInterfaceSubClass != info->bInterfaceSubClass)) {
			continue;
		}
		if ((id->match_flag_int_protocol) &&
		    (id->bInterfaceProtocol != info->bInterfaceProtocol)) {
			continue;
		}
		/* We found a match! */
		return (id);
	}

done:
	return (NULL);
}

/*------------------------------------------------------------------------*
 *	usbd_lookup_id_by_uaa - factored out code
 *
 * Return values:
 *    0: Success
 * Else: Failure
 *------------------------------------------------------------------------*/
int
usbd_lookup_id_by_uaa(const struct usb_device_id *id, usb_size_t sizeof_id,
    struct usb_attach_arg *uaa)
{
	id = usbd_lookup_id_by_info(id, sizeof_id, &uaa->info);
	if (id) {
		/* copy driver info */
		uaa->driver_info = id->driver_info;
		return (0);
	}
	return (ENXIO);
}

/*------------------------------------------------------------------------*
 *	Export the USB device ID format we use to userspace tools.
 *------------------------------------------------------------------------*/
#if BYTE_ORDER == BIG_ENDIAN
#define	U16_XOR "8"
#define	U32_XOR "12"
#define	U64_XOR "56"
#define	U8_BITFIELD_XOR "7"
#define	U16_BITFIELD_XOR "15"
#define	U32_BITFIELD_XOR "31"
#define	U64_BITFIELD_XOR "63"
#else
#define	U16_XOR "0"
#define	U32_XOR "0"
#define	U64_XOR "0"
#define	U8_BITFIELD_XOR "0"
#define	U16_BITFIELD_XOR "0"
#define	U32_BITFIELD_XOR "0"
#define	U64_BITFIELD_XOR "0"
#endif

#if USB_HAVE_COMPAT_LINUX
#define	MFL_SIZE "1"
#else
#define	MFL_SIZE "0"
#endif

#if defined(KLD_MODULE) && (USB_HAVE_ID_SECTION != 0)
static const char __section("bus_autoconf_format") __used usb_id_format[] = {

	/* Declare that three different sections use the same format */

	"usb_host_id{256,:}"
	"usb_device_id{256,:}"
	"usb_dual_id{256,:}"

	/* List size of fields in the usb_device_id structure */

#if ULONG_MAX >= 0xFFFFFFFFUL
	"unused{0,8}"
	"unused{0,8}"
	"unused{0,8}"
	"unused{0,8}"
#if ULONG_MAX >= 0xFFFFFFFFFFFFFFFFULL
	"unused{0,8}"
	"unused{0,8}"
	"unused{0,8}"
	"unused{0,8}"
#endif
#else
#error "Please update code."
#endif

	"idVendor[0]{" U16_XOR ",8}"
	"idVendor[1]{" U16_XOR ",8}"
	"idProduct[0]{" U16_XOR ",8}"
	"idProduct[1]{" U16_XOR ",8}"
	"bcdDevice_lo[0]{" U16_XOR ",8}"
	"bcdDevice_lo[1]{" U16_XOR ",8}"
	"bcdDevice_hi[0]{" U16_XOR ",8}"
	"bcdDevice_hi[1]{" U16_XOR ",8}"

	"bDeviceClass{0,8}"
	"bDeviceSubClass{0,8}"
	"bDeviceProtocol{0,8}"
	"bInterfaceClass{0,8}"
	"bInterfaceSubClass{0,8}"
	"bInterfaceProtocol{0,8}"

	"mf_vendor{" U8_BITFIELD_XOR ",1}"
	"mf_product{" U8_BITFIELD_XOR ",1}"
	"mf_dev_lo{" U8_BITFIELD_XOR ",1}"
	"mf_dev_hi{" U8_BITFIELD_XOR ",1}"

	"mf_dev_class{" U8_BITFIELD_XOR ",1}"
	"mf_dev_subclass{" U8_BITFIELD_XOR ",1}"
	"mf_dev_protocol{" U8_BITFIELD_XOR ",1}"
	"mf_int_class{" U8_BITFIELD_XOR ",1}"

	"mf_int_subclass{" U8_BITFIELD_XOR ",1}"
	"mf_int_protocol{" U8_BITFIELD_XOR ",1}"
	"unused{" U8_BITFIELD_XOR ",6}"

	"mfl_vendor{" U16_XOR "," MFL_SIZE "}"
	"mfl_product{" U16_XOR "," MFL_SIZE "}"
	"mfl_dev_lo{" U16_XOR "," MFL_SIZE "}"
	"mfl_dev_hi{" U16_XOR "," MFL_SIZE "}"

	"mfl_dev_class{" U16_XOR "," MFL_SIZE "}"
	"mfl_dev_subclass{" U16_XOR "," MFL_SIZE "}"
	"mfl_dev_protocol{" U16_XOR "," MFL_SIZE "}"
	"mfl_int_class{" U16_XOR "," MFL_SIZE "}"

	"mfl_int_subclass{" U16_XOR "," MFL_SIZE "}"
	"mfl_int_protocol{" U16_XOR "," MFL_SIZE "}"
	"unused{" U16_XOR "," MFL_SIZE "}"
	"unused{" U16_XOR "," MFL_SIZE "}"

	"unused{" U16_XOR "," MFL_SIZE "}"
	"unused{" U16_XOR "," MFL_SIZE "}"
	"unused{" U16_XOR "," MFL_SIZE "}"
	"unused{" U16_XOR "," MFL_SIZE "}"
};
#endif