
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-null.c,v 1.57 2006-03-23 14:58:44 hannes Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <pcap.h>
#include <stdio.h>
#include <string.h>

#include "interface.h"
#include "addrtoname.h"

#include "ip.h"
#ifdef INET6
#include "ip6.h"
#endif
#include "af.h"

/*
 * The DLT_NULL packet header is 4 bytes long. It contains a host-byte-order
 * 32-bit integer that specifies the family, e.g. AF_INET.
 *
 * Note here that "host" refers to the host on which the packets were
 * captured; that isn't necessarily *this* host.
 *
 * The OpenBSD DLT_LOOP packet header is the same, except that the integer
 * is in network byte order.
 */
#define	NULL_HDRLEN 4

/*
 * Byte-swap a 32-bit number.
 * ("htonl()" or "ntohl()" won't work - we want to byte-swap even on
 * big-endian platforms.)
 */
#define	SWAPLONG(y) \
((((y)&0xff)<<24) | (((y)&0xff00)<<8) | (((y)&0xff0000)>>8) | (((y)>>24)&0xff))

static inline void
null_hdr_print(u_int family, u_int length)
{
	if (!qflag) {
		(void)printf("AF %s (%u)",
			tok2str(bsd_af_values,"Unknown",family),family);
	} else {
		(void)printf("%s",
			tok2str(bsd_af_values,"Unknown AF %u",family));
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
null_if_print(const struct pcap_pkthdr *h, const u_char *p)
{
	u_int length = h->len;
	u_int caplen = h->caplen;
	u_int family;

	if (caplen < NULL_HDRLEN) {
		printf("[|null]");
		return (NULL_HDRLEN);
	}

	memcpy((char *)&family, (char *)p, sizeof(family));

	/*
	 * This isn't necessarily in our host byte order; if this is
	 * a DLT_LOOP capture, it's in network byte order, and if
	 * this is a DLT_NULL capture from a machine with the opposite
	 * byte-order, it's in the opposite byte order from ours.
	 *
	 * If the upper 16 bits aren't all zero, assume it's byte-swapped.
	 */
	if ((family & 0xFFFF0000) != 0)
		family = SWAPLONG(family);

	if (eflag)
		null_hdr_print(family, length);

	length -= NULL_HDRLEN;
	caplen -= NULL_HDRLEN;
	p += NULL_HDRLEN;

	switch (family) {

	case BSD_AFNUM_INET:
		ip_print(gndo, p, length);
		break;

#ifdef INET6
	case BSD_AFNUM_INET6_BSD:
	case BSD_AFNUM_INET6_FREEBSD:
	case BSD_AFNUM_INET6_DARWIN:
		ip6_print(gndo, p, length);
		break;
#endif

	case BSD_AFNUM_ISO:
		isoclns_print(p, length, caplen);
		break;

	case BSD_AFNUM_APPLETALK:
		atalk_print(p, length);
		break;

	case BSD_AFNUM_IPX:
		ipx_print(p, length);
		break;

	default:
		/* unknown AF_ value */
		if (!eflag)
			null_hdr_print(family, length + NULL_HDRLEN);
		if (!suppress_default_print)
			default_print(p, caplen);
	}

	return (NULL_HDRLEN);
}

/*
 * Local Variables:
 * c-style: whitesmith
 * c-basic-offset: 8
 * End:
 */