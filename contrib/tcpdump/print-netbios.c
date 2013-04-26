
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-netbios.c,v 1.20 2003-11-16 09:36:29 guy Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "interface.h"
#include "addrtoname.h"
#include "netbios.h"
#include "extract.h"

/*
 * Print NETBIOS packets.
 */
void
netbios_print(struct p8022Hdr *nb, u_int length)
{
	if (length < p8022Size) {
		(void)printf(" truncated-netbios %d", length);
		return;
	}

	if (nb->flags == UI) {
	    (void)printf("802.1 UI ");
	} else {
	    (void)printf("802.1 CONN ");
	}

	if ((u_char *)(nb + 1) > snapend) {
		printf(" [|netbios]");
		return;
	}

/*
	netbios_decode(nb, (u_char *)nb + p8022Size, length - p8022Size);
*/
}

#ifdef never
	(void)printf("%s.%d > ",
		     ipxaddr_string(EXTRACT_32BITS(ipx->srcNet), ipx->srcNode),
		     EXTRACT_16BITS(ipx->srcSkt));

	(void)printf("%s.%d:",
		     ipxaddr_string(EXTRACT_32BITS(ipx->dstNet), ipx->dstNode),
		     EXTRACT_16BITS(ipx->dstSkt));

	if ((u_char *)(ipx + 1) > snapend) {
		printf(" [|ipx]");
		return;
	}

	/* take length from ipx header */
	length = EXTRACT_16BITS(&ipx->length);

	ipx_decode(ipx, (u_char *)ipx + ipxSize, length - ipxSize);
#endif