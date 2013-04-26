
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

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/print-vjc.c,v 1.15 2004-03-25 03:31:17 mcr Exp $ (LBL)";
#endif

#include <tcpdump-stdinc.h>

#include <pcap.h>
#include <stdio.h>

#include "interface.h"
#include "addrtoname.h"

#include "slcompress.h"
#include "ppp.h"

/*
 * XXX - for BSD/OS PPP, what packets get supplied with a PPP header type
 * of PPP_VJC and what packets get supplied with a PPP header type of
 * PPP_VJNC?  PPP_VJNC is for "UNCOMPRESSED_TCP" packets, and PPP_VJC
 * is for COMPRESSED_TCP packets (PPP_IP is used for TYPE_IP packets).
 *
 * RFC 1144 implies that, on the wire, the packet type is *not* needed
 * for PPP, as different PPP protocol types can be used; it only needs
 * to be put on the wire for SLIP.
 *
 * It also indicates that, for compressed SLIP:
 *
 *	If the COMPRESSED_TCP bit is set in the first byte, it's
 *	a COMPRESSED_TCP packet; that byte is the change byte, and
 *	the COMPRESSED_TCP bit, 0x80, isn't used in the change byte.
 *
 *	If the upper 4 bits of the first byte are 7, it's an
 *	UNCOMPRESSED_TCP packet; that byte is the first byte of
 *	the UNCOMPRESSED_TCP modified IP header, with a connection
 *	number in the protocol field, and with the version field
 *	being 7, not 4.
 *
 *	Otherwise, the packet is an IPv4 packet (where the upper 4 bits
 *	of the packet are 4).
 *
 * So this routine looks as if it's sort-of intended to handle
 * compressed SLIP, although it doesn't handle UNCOMPRESSED_TCP
 * correctly for that (it doesn't fix the version number and doesn't
 * do anything to the protocol field), and doesn't check for COMPRESSED_TCP
 * packets correctly for that (you only check the first bit - see
 * B.1 in RFC 1144).
 *
 * But it's called for BSD/OS PPP, not SLIP - perhaps BSD/OS does weird
 * things with the headers?
 *
 * Without a BSD/OS VJC-compressed PPP trace, or knowledge of what the
 * BSD/OS VJC code does, we can't say what's the case.
 *
 * We therefore leave "proto" - which is the PPP protocol type - in place,
 * *not* marked as unused, for now, so that GCC warnings about the
 * unused argument remind us that we should fix this some day.
 */
int
vjc_print(register const char *bp, u_short proto _U_)
{
	int i;

	switch (bp[0] & 0xf0) {
	case TYPE_IP:
		if (eflag)
			printf("(vjc type=IP) ");
		return PPP_IP;
	case TYPE_UNCOMPRESSED_TCP:
		if (eflag)
			printf("(vjc type=raw TCP) ");
		return PPP_IP;
	case TYPE_COMPRESSED_TCP:
		if (eflag)
			printf("(vjc type=compressed TCP) ");
		for (i = 0; i < 8; i++) {
			if (bp[1] & (0x80 >> i))
				printf("%c", "?CI?SAWU"[i]);
		}
		if (bp[1])
			printf(" ");
		printf("C=0x%02x ", bp[2]);
		printf("sum=0x%04x ", *(u_short *)&bp[3]);
		return -1;
	case TYPE_ERROR:
		if (eflag)
			printf("(vjc type=error) ");
		return -1;
	default:
		if (eflag)
			printf("(vjc type=0x%02x) ", bp[0] & 0xf0);
		return -1;
	}
}