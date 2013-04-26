
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-token.c,v 1.27 2005-11-13 12:12:43 guy Exp $";
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
#include "ethertype.h"

#include "ether.h"
#include "token.h"

/* Extract src, dst addresses */
static inline void
extract_token_addrs(const struct token_header *trp, char *fsrc, char *fdst)
{
	memcpy(fdst, (const char *)trp->token_dhost, 6);
	memcpy(fsrc, (const char *)trp->token_shost, 6);
}

/*
 * Print the TR MAC header
 */
static inline void
token_hdr_print(register const struct token_header *trp, register u_int length,
	   register const u_char *fsrc, register const u_char *fdst)
{
	const char *srcname, *dstname;

	srcname = etheraddr_string(fsrc);
	dstname = etheraddr_string(fdst);

	if (vflag)
		(void) printf("%02x %02x %s %s %d: ",
		       trp->token_ac,
		       trp->token_fc,
		       srcname, dstname,
		       length);
	else
		printf("%s %s %d: ", srcname, dstname, length);
}

static const char *broadcast_indicator[] = {
	"Non-Broadcast", "Non-Broadcast",
	"Non-Broadcast", "Non-Broadcast",
	"All-routes",    "All-routes",
	"Single-route",  "Single-route"
};

static const char *direction[] = {
	"Forward", "Backward"
};

static const char *largest_frame[] = {
	"516",
	"1500",
	"2052",
	"4472",
	"8144",
	"11407",
	"17800",
	"??"
};

u_int
token_print(const u_char *p, u_int length, u_int caplen)
{
	const struct token_header *trp;
	u_short extracted_ethertype;
	struct ether_header ehdr;
	u_int route_len = 0, hdr_len = TOKEN_HDRLEN;
	int seg;

	trp = (const struct token_header *)p;

	if (caplen < TOKEN_HDRLEN) {
		printf("[|token-ring]");
		return hdr_len;
	}

	/*
	 * Get the TR addresses into a canonical form
	 */
	extract_token_addrs(trp, (char*)ESRC(&ehdr), (char*)EDST(&ehdr));

	/* Adjust for source routing information in the MAC header */
	if (IS_SOURCE_ROUTED(trp)) {
		/* Clear source-routed bit */
		*ESRC(&ehdr) &= 0x7f;

		if (eflag)
			token_hdr_print(trp, length, ESRC(&ehdr), EDST(&ehdr));

		if (caplen < TOKEN_HDRLEN + 2) {
			printf("[|token-ring]");
			return hdr_len;
		}
		route_len = RIF_LENGTH(trp);
		hdr_len += route_len;
		if (caplen < hdr_len) {
			printf("[|token-ring]");
			return hdr_len;
		}
		if (vflag) {
			printf("%s ", broadcast_indicator[BROADCAST(trp)]);
			printf("%s", direction[DIRECTION(trp)]);

			for (seg = 0; seg < SEGMENT_COUNT(trp); seg++)
				printf(" [%d:%d]", RING_NUMBER(trp, seg),
				    BRIDGE_NUMBER(trp, seg));
		} else {
			printf("rt = %x", EXTRACT_16BITS(&trp->token_rcf));

			for (seg = 0; seg < SEGMENT_COUNT(trp); seg++)
				printf(":%x", EXTRACT_16BITS(&trp->token_rseg[seg]));
		}
		printf(" (%s) ", largest_frame[LARGEST_FRAME(trp)]);
	} else {
		if (eflag)
			token_hdr_print(trp, length, ESRC(&ehdr), EDST(&ehdr));
	}

	/* Skip over token ring MAC header and routing information */
	length -= hdr_len;
	p += hdr_len;
	caplen -= hdr_len;

	/* Frame Control field determines interpretation of packet */
	if (FRAME_TYPE(trp) == TOKEN_FC_LLC) {
		/* Try to print the LLC-layer header & higher layers */
		if (llc_print(p, length, caplen, ESRC(&ehdr), EDST(&ehdr),
		    &extracted_ethertype) == 0) {
			/* ether_type not known, print raw packet */
			if (!eflag)
				token_hdr_print(trp,
				    length + TOKEN_HDRLEN + route_len,
				    ESRC(&ehdr), EDST(&ehdr));
			if (extracted_ethertype) {
				printf("(LLC %s) ",
			etherproto_string(htons(extracted_ethertype)));
			}
			if (!suppress_default_print)
				default_print(p, caplen);
		}
	} else {
		/* Some kinds of TR packet we cannot handle intelligently */
		/* XXX - dissect MAC packets if frame type is 0 */
		if (!eflag)
			token_hdr_print(trp, length + TOKEN_HDRLEN + route_len,
			    ESRC(&ehdr), EDST(&ehdr));
		if (!suppress_default_print)
			default_print(p, caplen);
	}
	return (hdr_len);
}

/*
 * This is the top level routine of the printer.  'p' points
 * to the TR header of the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 */
u_int
token_if_print(const struct pcap_pkthdr *h, const u_char *p)
{
	return (token_print(p, h->len, h->caplen));
}