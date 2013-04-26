
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
 * This file contains fallback-compatibility code for the old FreeBSD
 * USB stack.
 */
#ifdef HID_COMPAT7

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <dev/usb/usb.h>

#include "usbhid.h"
#include "usbvar.h"

int
hid_set_immed_compat7(int fd, int enable)
{
	return (ioctl(fd, USB_SET_IMMED, &enable));
}

int
hid_get_report_id_compat7(int fd)
{
	int temp = -1;

	if (ioctl(fd, USB_GET_REPORT_ID, &temp) < 0)
		return (-1);

	return (temp);
}

report_desc_t
hid_get_report_desc_compat7(int fd)
{
	struct usb_ctl_report_desc rep;

	rep.ucrd_size = 0;
	if (ioctl(fd, USB_GET_REPORT_DESC, &rep) < 0)
		return (NULL);

	return (hid_use_report_desc(rep.ucrd_data, (unsigned int)rep.ucrd_size));
}
#endif	/* HID_COMPAT7 */