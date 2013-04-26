
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-symantec.c,v 1.5 2005-07-07 01:22:21 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <pcap.h>

#include "interface.h"
#include "extract.h"
#include "addrtoname.h"
#include "ethertype.h"

#include "ether.h"

struct symantec_header {
	u_int8_t  stuff1[6];
	u_int16_t ether_type;
	u_int8_t  stuff2[36];
};

static inline void
symantec_hdr_print(register const u_char *bp, u_int length)
{
	register const struct symantec_header *sp;
	u_int16_t etype;

	sp = (const struct symantec_header *)bp;

	etype = EXTRACT_16BITS(&sp->ether_type);
	if (!qflag) {
	        if (etype <= ETHERMTU)
		          (void)printf("invalid ethertype %u", etype);
                else 
		          (void)printf("ethertype %s (0x%04x)",
				       tok2str(ethertype_values,"Unknown", etype),
                                       etype);
        } else {
                if (etype <= ETHERMTU)
                          (void)printf("invalid ethertype %u", etype);
                else 
                          (void)printf("%s", tok2str(ethertype_values,"Unknown Ethertype (0x%04x)", etype));  
        }

	(void)printf(", length %u: ", length);
}

/*
 * This is the top level routine of the printer.  'p' points
 * to the ether header of the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 */
u_int
symantec_if_print(const struct pcap_pkthdr *h, const u_char *p)
{
	u_int length = h->len;
	u_int caplen = h->caplen;
	struct symantec_header *sp;
	u_short ether_type;

	if (caplen < sizeof (struct symantec_header)) {
		printf("[|symantec]");
		return caplen;
	}

	if (eflag)
		symantec_hdr_print(p, length);

	length -= sizeof (struct symantec_header);
	caplen -= sizeof (struct symantec_header);
	sp = (struct symantec_header *)p;
	p += sizeof (struct symantec_header);

	ether_type = EXTRACT_16BITS(&sp->ether_type);

	if (ether_type <= ETHERMTU) {
		/* ether_type not known, print raw packet */
		if (!eflag)
			symantec_hdr_print((u_char *)sp, length + sizeof (struct symantec_header));

		if (!suppress_default_print)
			default_print(p, caplen);
	} else if (ethertype_print(gndo, ether_type, p, length, caplen) == 0) {
		/* ether_type not known, print raw packet */
		if (!eflag)
			symantec_hdr_print((u_char *)sp, length + sizeof (struct symantec_header));

		if (!suppress_default_print)
			default_print(p, caplen);
	} 

	return (sizeof (struct symantec_header));
}