
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-frag6.c,v 1.20 2005-04-20 22:33:06 guy Exp $";
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
frag6_print(register const u_char *bp, register const u_char *bp2)
{
	register const struct ip6_frag *dp;
	register const struct ip6_hdr *ip6;

	dp = (const struct ip6_frag *)bp;
	ip6 = (const struct ip6_hdr *)bp2;

	TCHECK(dp->ip6f_offlg);

	if (vflag) {
		printf("frag (0x%08x:%d|%ld)",
		       EXTRACT_32BITS(&dp->ip6f_ident),
		       EXTRACT_16BITS(&dp->ip6f_offlg) & IP6F_OFF_MASK,
		       sizeof(struct ip6_hdr) + EXTRACT_16BITS(&ip6->ip6_plen) -
			       (long)(bp - bp2) - sizeof(struct ip6_frag));
	} else {
		printf("frag (%d|%ld)",
		       EXTRACT_16BITS(&dp->ip6f_offlg) & IP6F_OFF_MASK,
		       sizeof(struct ip6_hdr) + EXTRACT_16BITS(&ip6->ip6_plen) -
			       (long)(bp - bp2) - sizeof(struct ip6_frag));
	}

#if 1
	/* it is meaningless to decode non-first fragment */
	if ((EXTRACT_16BITS(&dp->ip6f_offlg) & IP6F_OFF_MASK) != 0)
		return -1;
	else
#endif
	{
		fputs(" ", stdout);
		return sizeof(struct ip6_frag);
	}
trunc:
	fputs("[|frag]", stdout);
	return -1;
#undef TCHECK
}
#endif /* INET6 */