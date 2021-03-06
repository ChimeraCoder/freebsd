
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-rt6.c,v 1.27 2005-04-20 22:34:57 guy Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef INET6

#include <tcpdump-stdinc.h>

#include <stdio.h>

#include "ip6.h"

#include "interface.h"
#include "addrtoname.h"
#include "extract.h"

int
rt6_print(register const u_char *bp, const u_char *bp2 _U_)
{
	register const struct ip6_rthdr *dp;
	register const struct ip6_rthdr0 *dp0;
	register const u_char *ep;
	int i, len;
	register const struct in6_addr *addr;

	dp = (struct ip6_rthdr *)bp;
	len = dp->ip6r_len;

	/* 'ep' points to the end of available data. */
	ep = snapend;

	TCHECK(dp->ip6r_segleft);

	printf("srcrt (len=%d", dp->ip6r_len);	/*)*/
	printf(", type=%d", dp->ip6r_type);
	printf(", segleft=%d", dp->ip6r_segleft);

	switch (dp->ip6r_type) {
#ifndef IPV6_RTHDR_TYPE_0
#define IPV6_RTHDR_TYPE_0 0
#endif
#ifndef IPV6_RTHDR_TYPE_2
#define IPV6_RTHDR_TYPE_2 2
#endif
	case IPV6_RTHDR_TYPE_0:
	case IPV6_RTHDR_TYPE_2:			/* Mobile IPv6 ID-20 */
		dp0 = (struct ip6_rthdr0 *)dp;

		TCHECK(dp0->ip6r0_reserved);
		if (dp0->ip6r0_reserved || vflag) {
			printf(", rsv=0x%0x",
			    EXTRACT_32BITS(&dp0->ip6r0_reserved));
		}

		if (len % 2 == 1)
			goto trunc;
		len >>= 1;
		addr = &dp0->ip6r0_addr[0];
		for (i = 0; i < len; i++) {
			if ((u_char *)(addr + 1) > ep)
				goto trunc;

			printf(", [%d]%s", i, ip6addr_string(addr));
			addr++;
		}
		/*(*/
		printf(") ");
		return((dp0->ip6r0_len + 1) << 3);
		break;
	default:
		goto trunc;
		break;
	}

 trunc:
	fputs("[|srcrt]", stdout);
	return -1;
}
#endif /* INET6 */