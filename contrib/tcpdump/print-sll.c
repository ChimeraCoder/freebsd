
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-sll.c,v 1.19 2005-11-13 12:12:43 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <string.h>
#include <pcap.h>

#include "interface.h"
#include "addrtoname.h"
#include "ethertype.h"
#include "extract.h"

#include "ether.h"
#include "sll.h"

const struct tok sll_pkttype_values[] = {
    { LINUX_SLL_HOST, "In" },
    { LINUX_SLL_BROADCAST, "B" },
    { LINUX_SLL_MULTICAST, "M" },
    { LINUX_SLL_OTHERHOST, "P" },
    { LINUX_SLL_OUTGOING, "Out" },
    { 0, NULL}
};

static inline void
sll_print(register const struct sll_header *sllp, u_int length)
{
	u_short ether_type;

        printf("%3s ",tok2str(sll_pkttype_values,"?",EXTRACT_16BITS(&sllp->sll_pkttype)));

	/*
	 * XXX - check the link-layer address type value?
	 * For now, we just assume 6 means Ethernet.
	 * XXX - print others as strings of hex?
	 */
	if (EXTRACT_16BITS(&sllp->sll_halen) == 6)
		(void)printf("%s ", etheraddr_string(sllp->sll_addr));

	if (!qflag) {
		ether_type = EXTRACT_16BITS(&sllp->sll_protocol);
	
		if (ether_type <= ETHERMTU) {
			/*
			 * Not an Ethernet type; what type is it?
			 */
			switch (ether_type) {

			case LINUX_SLL_P_802_3:
				/*
				 * Ethernet_802.3 IPX frame.
				 */
				(void)printf("802.3");
				break;

			case LINUX_SLL_P_802_2:
				/*
				 * 802.2.
				 */
				(void)printf("802.2");
				break;

			default:
				/*
				 * What is it?
				 */
				(void)printf("ethertype Unknown (0x%04x)",
				    ether_type);
				break;
			}
		} else {
			(void)printf("ethertype %s (0x%04x)",
			    tok2str(ethertype_values, "Unknown", ether_type),
			    ether_type);
		}
		(void)printf(", length %u: ", length);
	}
}

/*
 * This is the top level routine of the printer.  'p' points to the
 * Linux "cooked capture" header of the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 */
u_int
sll_if_print(const struct pcap_pkthdr *h, const u_char *p)
{
	u_int caplen = h->caplen;
	u_int length = h->len;
	register const struct sll_header *sllp;
	u_short ether_type;
	u_short extracted_ethertype;

	if (caplen < SLL_HDR_LEN) {
		/*
		 * XXX - this "can't happen" because "pcap-linux.c" always
		 * adds this many bytes of header to every packet in a
		 * cooked socket capture.
		 */
		printf("[|sll]");
		return (caplen);
	}

	sllp = (const struct sll_header *)p;

	if (eflag)
		sll_print(sllp, length);

	/*
	 * Go past the cooked-mode header.
	 */
	length -= SLL_HDR_LEN;
	caplen -= SLL_HDR_LEN;
	p += SLL_HDR_LEN;

	ether_type = EXTRACT_16BITS(&sllp->sll_protocol);

recurse:
	/*
	 * Is it (gag) an 802.3 encapsulation, or some non-Ethernet
	 * packet type?
	 */
	if (ether_type <= ETHERMTU) {
		/*
		 * Yes - what type is it?
		 */
		switch (ether_type) {

		case LINUX_SLL_P_802_3:
			/*
			 * Ethernet_802.3 IPX frame.
			 */
			ipx_print(p, length);
			break;

		case LINUX_SLL_P_802_2:
			/*
			 * 802.2.
			 * Try to print the LLC-layer header & higher layers.
			 */
			if (llc_print(p, length, caplen, NULL, NULL,
			    &extracted_ethertype) == 0)
				goto unknown;	/* unknown LLC type */
			break;

		default:
			extracted_ethertype = 0;
			/*FALLTHROUGH*/

		unknown:
			/* ether_type not known, print raw packet */
			if (!eflag)
				sll_print(sllp, length + SLL_HDR_LEN);
			if (extracted_ethertype) {
				printf("(LLC %s) ",
			       etherproto_string(htons(extracted_ethertype)));
			}
			if (!suppress_default_print)
				default_print(p, caplen);
			break;
		}
	} else if (ether_type == ETHERTYPE_8021Q) {
		/*
		 * Print VLAN information, and then go back and process
		 * the enclosed type field.
		 */
		if (caplen < 4 || length < 4) {
			printf("[|vlan]");
			return (SLL_HDR_LEN);
		}
	        if (eflag) {
	        	u_int16_t tag = EXTRACT_16BITS(p);

			printf("vlan %u, p %u%s, ",
			    tag & 0xfff,
			    tag >> 13,
			    (tag & 0x1000) ? ", CFI" : "");
		}

		ether_type = EXTRACT_16BITS(p + 2);
		if (ether_type <= ETHERMTU)
			ether_type = LINUX_SLL_P_802_2;
		if (!qflag) {
			(void)printf("ethertype %s, ",
			    tok2str(ethertype_values, "Unknown", ether_type));
		}
		p += 4;
		length -= 4;
		caplen -= 4;
		goto recurse;
	} else {
		if (ethertype_print(gndo, ether_type, p, length, caplen) == 0) {
			/* ether_type not known, print raw packet */
			if (!eflag)
				sll_print(sllp, length + SLL_HDR_LEN);
			if (!suppress_default_print)
				default_print(p, caplen);
		}
	}

	return (SLL_HDR_LEN);
}