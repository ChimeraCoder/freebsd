
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-chdlc.c,v 1.43 2005-11-29 08:56:19 hannes Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <pcap.h>
#include <stdio.h>

#include "interface.h"
#include "addrtoname.h"
#include "ethertype.h"
#include "extract.h"
#include "ppp.h"
#include "chdlc.h"

static void chdlc_slarp_print(const u_char *, u_int);

const struct tok chdlc_cast_values[] = { 
    { CHDLC_UNICAST, "unicast" },
    { CHDLC_BCAST, "bcast" },
    { 0, NULL}
};


/* Standard CHDLC printer */
u_int
chdlc_if_print(const struct pcap_pkthdr *h, register const u_char *p)
{
	register u_int length = h->len;
	register u_int caplen = h->caplen;

	if (caplen < CHDLC_HDRLEN) {
		printf("[|chdlc]");
		return (caplen);
	}
        return (chdlc_print(p,length));
}

u_int
chdlc_print(register const u_char *p, u_int length) {
	u_int proto;

	proto = EXTRACT_16BITS(&p[2]);
	if (eflag) {
                printf("%s, ethertype %s (0x%04x), length %u: ",
                       tok2str(chdlc_cast_values, "0x%02x", p[0]),
                       tok2str(ethertype_values, "Unknown", proto),
                       proto,
                       length);
	}

	length -= CHDLC_HDRLEN;
	p += CHDLC_HDRLEN;

	switch (proto) {
	case ETHERTYPE_IP:
		ip_print(gndo, p, length);
		break;
#ifdef INET6
	case ETHERTYPE_IPV6:
		ip6_print(gndo, p, length);
		break;
#endif
	case CHDLC_TYPE_SLARP:
		chdlc_slarp_print(p, length);
		break;
#if 0
	case CHDLC_TYPE_CDP:
		chdlc_cdp_print(p, length);
		break;
#endif
        case ETHERTYPE_MPLS:
        case ETHERTYPE_MPLS_MULTI:
                mpls_print(p, length);
		break;
        case ETHERTYPE_ISO:
                /* is the fudge byte set ? lets verify by spotting ISO headers */
                if (*(p+1) == 0x81 ||
                    *(p+1) == 0x82 ||
                    *(p+1) == 0x83)
                    isoclns_print(p+1, length-1, length-1);
                else
                    isoclns_print(p, length, length);
                break;
	default:
                if (!eflag)
                        printf("unknown CHDLC protocol (0x%04x)", proto);
                break;
	}

	return (CHDLC_HDRLEN);
}

/*
 * The fixed-length portion of a SLARP packet.
 */
struct cisco_slarp {
	u_int8_t code[4];
#define SLARP_REQUEST	0
#define SLARP_REPLY	1
#define SLARP_KEEPALIVE	2
	union {
		struct {
			u_int8_t addr[4];
			u_int8_t mask[4];
		} addr;
		struct {
			u_int8_t myseq[4];
			u_int8_t yourseq[4];
			u_int8_t rel[2];
		} keep;
	} un;
};

#define SLARP_MIN_LEN	14
#define SLARP_MAX_LEN	18

static void
chdlc_slarp_print(const u_char *cp, u_int length)
{
	const struct cisco_slarp *slarp;
        u_int sec,min,hrs,days;

        printf("SLARP (length: %u), ",length);
	if (length < SLARP_MIN_LEN)
		goto trunc;

	slarp = (const struct cisco_slarp *)cp;
	TCHECK2(*slarp, SLARP_MIN_LEN);
	switch (EXTRACT_32BITS(&slarp->code)) {
	case SLARP_REQUEST:
		printf("request");
		/*
		 * At least according to William "Chops" Westfield's
		 * message in
		 *
		 *	http://www.nethelp.no/net/cisco-hdlc.txt
		 *
		 * the address and mask aren't used in requests -
		 * they're just zero.
		 */
		break;
	case SLARP_REPLY:
		printf("reply %s/%s",
			ipaddr_string(&slarp->un.addr.addr),
			ipaddr_string(&slarp->un.addr.mask));
		break;
	case SLARP_KEEPALIVE:
		printf("keepalive: mineseen=0x%08x, yourseen=0x%08x, reliability=0x%04x",
                       EXTRACT_32BITS(&slarp->un.keep.myseq),
                       EXTRACT_32BITS(&slarp->un.keep.yourseq),
                       EXTRACT_16BITS(&slarp->un.keep.rel));

                if (length >= SLARP_MAX_LEN) { /* uptime-stamp is optional */
                        cp += SLARP_MIN_LEN;
                        if (!TTEST2(*cp, 4))
                                goto trunc;
                        sec = EXTRACT_32BITS(cp) / 1000;
                        min = sec / 60; sec -= min * 60;
                        hrs = min / 60; min -= hrs * 60;
                        days = hrs / 24; hrs -= days * 24;
                        printf(", link uptime=%ud%uh%um%us",days,hrs,min,sec);
                }
		break;
	default:
		printf("0x%02x unknown", EXTRACT_32BITS(&slarp->code));
                if (vflag <= 1)
                    print_unknown_data(cp+4,"\n\t",length-4);
		break;
	}

	if (SLARP_MAX_LEN < length && vflag)
		printf(", (trailing junk: %d bytes)", length - SLARP_MAX_LEN);
        if (vflag > 1)
            print_unknown_data(cp+4,"\n\t",length-4);
	return;

trunc:
	printf("[|slarp]");
}


/*
 * Local Variables:
 * c-style: whitesmith
 * c-basic-offset: 8
 * End:
 */