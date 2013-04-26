
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <pcap.h>
#include <stdio.h>
#include <string.h>

#include "interface.h"


#if defined(HAVE_PCAP_USB_H) && defined(DLT_USB_LINUX)
#include <pcap/usb.h>

/* returns direction: 1=inbound 2=outbound -1=invalid */
static int
get_direction(int transfer_type, int event_type)
{
	int direction;

	direction = -1;
	switch(transfer_type){
	case URB_BULK:
	case URB_CONTROL:
	case URB_ISOCHRONOUS:
		switch(event_type)
		{
		case URB_SUBMIT:
			direction = 2;
			break;
		case URB_COMPLETE:
		case URB_ERROR:
			direction = 1;
			break;
		default:
			direction = -1;
		}
		break;
	case URB_INTERRUPT:
		switch(event_type)
		{
		case URB_SUBMIT:
			direction = 1;
			break;
		case URB_COMPLETE:
		case URB_ERROR:
			direction = 2;
			break;
		default:
			direction = -1;
		}
		break;
	 default:
		direction = -1;
	}

	return direction;
}

static void
usb_header_print(const pcap_usb_header *uh)
{
	int direction;

	switch(uh->transfer_type)
	{
		case URB_ISOCHRONOUS:
			printf("ISOCHRONOUS");
			break;
		case URB_INTERRUPT:
			printf("INTERRUPT");
			break;
		case URB_CONTROL:
			printf("CONTROL");
			break;
		case URB_BULK:
			printf("BULK");
			break;
		default:
			printf(" ?");
	}

	switch(uh->event_type)
	{
		case URB_SUBMIT:
			printf(" SUBMIT");
			break;
		case URB_COMPLETE:
			printf(" COMPLETE");
			break;
		case URB_ERROR:
			printf(" ERROR");
			break;
		default:
			printf(" ?");
	}

	direction = get_direction(uh->transfer_type, uh->event_type);
	if(direction == 1)
		printf(" from");
	else if(direction == 2)
		printf(" to");
	printf(" %d:%d:%d", uh->bus_id, uh->device_address, uh->endpoint_number & 0x7f);
}

/*
 * This is the top level routine of the printer for captures with a
 * 48-byte header.
 *
 * 'p' points to the header of the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 */
u_int
usb_linux_48_byte_print(const struct pcap_pkthdr *h, register const u_char *p)
{
	if (h->caplen < sizeof(pcap_usb_header)) {
		printf("[|usb]");
		return(sizeof(pcap_usb_header));
	}

	usb_header_print((const pcap_usb_header *) p);

	return(sizeof(pcap_usb_header));
}

#ifdef DLT_USB_LINUX_MMAPPED
/*
 * This is the top level routine of the printer for captures with a
 * 64-byte header.
 *
 * 'p' points to the header of the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 */
u_int
usb_linux_64_byte_print(const struct pcap_pkthdr *h, register const u_char *p)
{
	if (h->caplen < sizeof(pcap_usb_header_mmapped)) {
		printf("[|usb]");
		return(sizeof(pcap_usb_header_mmapped));
	}

	usb_header_print((const pcap_usb_header *) p);

	return(sizeof(pcap_usb_header_mmapped));
}
#endif /* DLT_USB_LINUX_MMAPPED */

#endif /* defined(HAVE_PCAP_USB_H) && defined(DLT_USB_LINUX) */