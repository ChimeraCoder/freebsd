
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
#endif			/* USB_GLOBAL_INCLUDE_FILE */

static const char* usb_errstr_table[USB_ERR_MAX] = {
	[USB_ERR_NORMAL_COMPLETION]	= "USB_ERR_NORMAL_COMPLETION",
	[USB_ERR_PENDING_REQUESTS]	= "USB_ERR_PENDING_REQUESTS",
	[USB_ERR_NOT_STARTED]		= "USB_ERR_NOT_STARTED",
	[USB_ERR_INVAL]			= "USB_ERR_INVAL",
	[USB_ERR_NOMEM]			= "USB_ERR_NOMEM",
	[USB_ERR_CANCELLED]		= "USB_ERR_CANCELLED",
	[USB_ERR_BAD_ADDRESS]		= "USB_ERR_BAD_ADDRESS",
	[USB_ERR_BAD_BUFSIZE]		= "USB_ERR_BAD_BUFSIZE",
	[USB_ERR_BAD_FLAG]		= "USB_ERR_BAD_FLAG",
	[USB_ERR_NO_CALLBACK]		= "USB_ERR_NO_CALLBACK",
	[USB_ERR_IN_USE]		= "USB_ERR_IN_USE",
	[USB_ERR_NO_ADDR]		= "USB_ERR_NO_ADDR",
	[USB_ERR_NO_PIPE]		= "USB_ERR_NO_PIPE",
	[USB_ERR_ZERO_NFRAMES]		= "USB_ERR_ZERO_NFRAMES",
	[USB_ERR_ZERO_MAXP]		= "USB_ERR_ZERO_MAXP",
	[USB_ERR_SET_ADDR_FAILED]	= "USB_ERR_SET_ADDR_FAILED",
	[USB_ERR_NO_POWER]		= "USB_ERR_NO_POWER",
	[USB_ERR_TOO_DEEP]		= "USB_ERR_TOO_DEEP",
	[USB_ERR_IOERROR]		= "USB_ERR_IOERROR",
	[USB_ERR_NOT_CONFIGURED]	= "USB_ERR_NOT_CONFIGURED",
	[USB_ERR_TIMEOUT]		= "USB_ERR_TIMEOUT",
	[USB_ERR_SHORT_XFER]		= "USB_ERR_SHORT_XFER",
	[USB_ERR_STALLED]		= "USB_ERR_STALLED",
	[USB_ERR_INTERRUPTED]		= "USB_ERR_INTERRUPTED",
	[USB_ERR_DMA_LOAD_FAILED]	= "USB_ERR_DMA_LOAD_FAILED",
	[USB_ERR_BAD_CONTEXT]		= "USB_ERR_BAD_CONTEXT",
	[USB_ERR_NO_ROOT_HUB]		= "USB_ERR_NO_ROOT_HUB",
	[USB_ERR_NO_INTR_THREAD]	= "USB_ERR_NO_INTR_THREAD",
	[USB_ERR_NOT_LOCKED]		= "USB_ERR_NOT_LOCKED",
};

/*------------------------------------------------------------------------*
 *	usbd_errstr
 *
 * This function converts an USB error code into a string.
 *------------------------------------------------------------------------*/
const char *
usbd_errstr(usb_error_t err)
{
	return (err < USB_ERR_MAX ? usb_errstr_table[err] : "USB_ERR_UNKNOWN");
}