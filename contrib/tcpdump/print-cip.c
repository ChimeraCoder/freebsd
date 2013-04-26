
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-cip.c,v 1.26 2005-07-07 01:22:17 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <pcap.h>

#include "interface.h"
#include "addrtoname.h"
#include "ethertype.h"
#include "ether.h"

#define RFC1483LLC_LEN	8

static unsigned char rfcllc[] = {
	0xaa,	/* DSAP: non-ISO */
	0xaa,	/* SSAP: non-ISO */
	0x03,	/* Ctrl: Unnumbered Information Command PDU */
	0x00,	/* OUI: EtherType */
	0x00,
	0x00 };

static inline void
cip_print(int length)
{
	/*
	 * There is no MAC-layer header, so just print the length.
	 */
	printf("%d: ", length);
}

/*
 * This is the top level routine of the printer.  'p' points
 * to the LLC/SNAP or raw header of the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 */
u_int
cip_if_print(const struct pcap_pkthdr *h, const u_char *p)
{
	u_int caplen = h->caplen;
	u_int length = h->len;
	u_short extracted_ethertype;

	if (memcmp(rfcllc, p, sizeof(rfcllc))==0 && caplen < RFC1483LLC_LEN) {
		printf("[|cip]");
		return (0);
	}

	if (eflag)
		cip_print(length);

	if (memcmp(rfcllc, p, sizeof(rfcllc)) == 0) {
		/*
		 * LLC header is present.  Try to print it & higher layers.
		 */
		if (llc_print(p, length, caplen, NULL, NULL,
		    &extracted_ethertype) == 0) {
			/* ether_type not known, print raw packet */
			if (!eflag)
				cip_print(length);
			if (extracted_ethertype) {
				printf("(LLC %s) ",
			       etherproto_string(htons(extracted_ethertype)));
			}
			if (!suppress_default_print)
				default_print(p, caplen);
		}
	} else {
		/*
		 * LLC header is absent; treat it as just IP.
		 */
		ip_print(gndo, p, length);
	}

	return (0);
}


/*
 * Local Variables:
 * c-style: whitesmith
 * c-basic-offset: 8
 * End:
 */