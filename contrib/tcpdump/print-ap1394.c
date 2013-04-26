
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-ap1394.c,v 1.5 2006-02-11 22:12:06 hannes Exp $ (LBL)";
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

/*
 * Structure of a header for Apple's IP-over-IEEE 1384 BPF header.
 */
#define FIREWIRE_EUI64_LEN	8
struct firewire_header {
	u_char  firewire_dhost[FIREWIRE_EUI64_LEN];
	u_char  firewire_shost[FIREWIRE_EUI64_LEN];
	u_short firewire_type;
};

/*
 * Length of that header; note that some compilers may pad
 * "struct firewire_header" to a multiple of 4 bytes, for example, so
 * "sizeof (struct firewire_header)" may not give the right answer.
 */
#define FIREWIRE_HDRLEN		18

static inline void
ap1394_hdr_print(register const u_char *bp, u_int length)
{
	register const struct firewire_header *fp;
	u_int16_t firewire_type;

	fp = (const struct firewire_header *)bp;

	(void)printf("%s > %s",
		     linkaddr_string(fp->firewire_dhost, LINKADDR_IEEE1394, FIREWIRE_EUI64_LEN),
		     linkaddr_string(fp->firewire_shost, LINKADDR_IEEE1394, FIREWIRE_EUI64_LEN));

	firewire_type = EXTRACT_16BITS(&fp->firewire_type);
	if (!qflag) {
		(void)printf(", ethertype %s (0x%04x)",
			       tok2str(ethertype_values,"Unknown", firewire_type),
                               firewire_type);
        } else {
                (void)printf(", %s", tok2str(ethertype_values,"Unknown Ethertype (0x%04x)", firewire_type));
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
ap1394_if_print(const struct pcap_pkthdr *h, const u_char *p)
{
	u_int length = h->len;
	u_int caplen = h->caplen;
	struct firewire_header *fp;
	u_short ether_type;

	if (caplen < FIREWIRE_HDRLEN) {
		printf("[|ap1394]");
		return FIREWIRE_HDRLEN;
	}

	if (eflag)
		ap1394_hdr_print(p, length);

	length -= FIREWIRE_HDRLEN;
	caplen -= FIREWIRE_HDRLEN;
	fp = (struct firewire_header *)p;
	p += FIREWIRE_HDRLEN;

	ether_type = EXTRACT_16BITS(&fp->firewire_type);
	if (ethertype_print(gndo, ether_type, p, length, caplen) == 0) {
		/* ether_type not known, print raw packet */
		if (!eflag)
			ap1394_hdr_print((u_char *)fp, length + FIREWIRE_HDRLEN);

		if (!suppress_default_print)
			default_print(p, caplen);
	} 

	return FIREWIRE_HDRLEN;
}