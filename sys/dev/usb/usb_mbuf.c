
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
#include <dev/usb/usb_dev.h>
#include <dev/usb/usb_mbuf.h>
#endif			/* USB_GLOBAL_INCLUDE_FILE */

/*------------------------------------------------------------------------*
 *      usb_alloc_mbufs - allocate mbufs to an usbd interface queue
 *
 * Returns:
 *   A pointer that should be passed to "free()" when the buffer(s)
 *   should be released.
 *------------------------------------------------------------------------*/
void   *
usb_alloc_mbufs(struct malloc_type *type, struct usb_ifqueue *ifq,
    usb_size_t block_size, uint16_t nblocks)
{
	struct usb_mbuf *m_ptr;
	uint8_t *data_ptr;
	void *free_ptr = NULL;
	usb_size_t alloc_size;

	/* align data */
	block_size += ((-block_size) & (USB_HOST_ALIGN - 1));

	if (nblocks && block_size) {

		alloc_size = (block_size + sizeof(struct usb_mbuf)) * nblocks;

		free_ptr = malloc(alloc_size, type, M_WAITOK | M_ZERO);

		if (free_ptr == NULL) {
			goto done;
		}
		m_ptr = free_ptr;
		data_ptr = (void *)(m_ptr + nblocks);

		while (nblocks--) {

			m_ptr->cur_data_ptr =
			    m_ptr->min_data_ptr = data_ptr;

			m_ptr->cur_data_len =
			    m_ptr->max_data_len = block_size;

			USB_IF_ENQUEUE(ifq, m_ptr);

			m_ptr++;
			data_ptr += block_size;
		}
	}
done:
	return (free_ptr);
}