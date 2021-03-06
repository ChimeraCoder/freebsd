
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-ripng.c,v 1.18 2005-01-04 00:15:54 guy Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef INET6

#include <tcpdump-stdinc.h>
#include <stdio.h>

#include "route6d.h"
#include "interface.h"
#include "addrtoname.h"
#include "extract.h"

#if !defined(IN6_IS_ADDR_UNSPECIFIED) && !defined(_MSC_VER) /* MSVC inline */
static int IN6_IS_ADDR_UNSPECIFIED(const struct in6_addr *addr)
{
    static const struct in6_addr in6addr_any;        /* :: */
    return (memcmp(addr, &in6addr_any, sizeof(*addr)) == 0);
}
#endif

static int
rip6_entry_print(register const struct netinfo6 *ni, int metric)
{
	int l;
	l = printf("%s/%d", ip6addr_string(&ni->rip6_dest), ni->rip6_plen);
	if (ni->rip6_tag)
		l += printf(" [%d]", EXTRACT_16BITS(&ni->rip6_tag));
	if (metric)
		l += printf(" (%d)", ni->rip6_metric);
	return l;
}

void
ripng_print(const u_char *dat, unsigned int length)
{
	register const struct rip6 *rp = (struct rip6 *)dat;
	register const struct netinfo6 *ni;
	register u_int amt;
	register u_int i;
	int j;
	int trunc;

	if (snapend < dat)
		return;
	amt = snapend - dat;
	i = min(length, amt);
	if (i < (sizeof(struct rip6) - sizeof(struct netinfo6)))
		return;
	i -= (sizeof(struct rip6) - sizeof(struct netinfo6));

	switch (rp->rip6_cmd) {

	case RIP6_REQUEST:
		j = length / sizeof(*ni);
		if (j == 1
		    &&  rp->rip6_nets->rip6_metric == HOPCNT_INFINITY6
		    &&  IN6_IS_ADDR_UNSPECIFIED(&rp->rip6_nets->rip6_dest)) {
			printf(" ripng-req dump");
			break;
		}
		if (j * sizeof(*ni) != length - 4)
			printf(" ripng-req %d[%u]:", j, length);
		else
			printf(" ripng-req %d:", j);
		trunc = ((i / sizeof(*ni)) * sizeof(*ni) != i);
		for (ni = rp->rip6_nets; i >= sizeof(*ni);
		    i -= sizeof(*ni), ++ni) {
			if (vflag > 1)
				printf("\n\t");
			else
				printf(" ");
			rip6_entry_print(ni, 0);
		}
		break;
	case RIP6_RESPONSE:
		j = length / sizeof(*ni);
		if (j * sizeof(*ni) != length - 4)
			printf(" ripng-resp %d[%u]:", j, length);
		else
			printf(" ripng-resp %d:", j);
		trunc = ((i / sizeof(*ni)) * sizeof(*ni) != i);
		for (ni = rp->rip6_nets; i >= sizeof(*ni);
		    i -= sizeof(*ni), ++ni) {
			if (vflag > 1)
				printf("\n\t");
			else
				printf(" ");
			rip6_entry_print(ni, ni->rip6_metric);
		}
		if (trunc)
			printf("[|ripng]");
		break;
	default:
		printf(" ripng-%d ?? %u", rp->rip6_cmd, length);
		break;
	}
	if (rp->rip6_vers != RIP6_VERSION)
		printf(" [vers %d]", rp->rip6_vers);
}
#endif /* INET6 */