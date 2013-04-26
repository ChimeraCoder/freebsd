
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

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/print-bt.c,v 1.2 2008-09-25 21:45:50 guy Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <pcap.h>
#include <stdio.h>
#include <string.h>

#include "interface.h"
#include "extract.h"
#include "addrtoname.h"

#if defined(DLT_BLUETOOTH_HCI_H4_WITH_PHDR) && defined(HAVE_PCAP_BLUETOOTH_H)
#include <pcap/bluetooth.h>

#define	BT_HDRLEN sizeof(pcap_bluetooth_h4_header)
/*
 * This is the top level routine of the printer.  'p' points
 * to the bluetooth header of the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 */
u_int
bt_if_print(const struct pcap_pkthdr *h, const u_char *p)
{
	u_int length = h->len;
	u_int caplen = h->caplen;
	const pcap_bluetooth_h4_header* hdr = (const pcap_bluetooth_h4_header*)p;

	if (caplen < BT_HDRLEN) {
		printf("[|bt]");
		return (BT_HDRLEN);
	}
	caplen -= BT_HDRLEN;
	length -= BT_HDRLEN;
	p += BT_HDRLEN;
	if (eflag)
		(void)printf("hci length %d, direction %s, ", length, (EXTRACT_32BITS(&hdr->direction)&0x1)?"in":"out");

	if (!suppress_default_print)
		default_print(p, caplen);

	return (BT_HDRLEN);
}
#endif


/*
 * Local Variables:
 * c-style: whitesmith
 * c-basic-offset: 8
 * End:
 */