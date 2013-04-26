
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_inet.h"
#include "opt_inet6.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet/icmp6.h>

/*
 * Destination options header processing.
 */
int
dest6_input(struct mbuf **mp, int *offp, int proto)
{
	struct mbuf *m = *mp;
	int off = *offp, dstoptlen, optlen;
	struct ip6_dest *dstopts;
	u_int8_t *opt;

	/* validation of the length of the header */
#ifndef PULLDOWN_TEST
	IP6_EXTHDR_CHECK(m, off, sizeof(*dstopts), IPPROTO_DONE);
	dstopts = (struct ip6_dest *)(mtod(m, caddr_t) + off);
#else
	IP6_EXTHDR_GET(dstopts, struct ip6_dest *, m, off, sizeof(*dstopts));
	if (dstopts == NULL)
		return IPPROTO_DONE;
#endif
	dstoptlen = (dstopts->ip6d_len + 1) << 3;

#ifndef PULLDOWN_TEST
	IP6_EXTHDR_CHECK(m, off, dstoptlen, IPPROTO_DONE);
	dstopts = (struct ip6_dest *)(mtod(m, caddr_t) + off);
#else
	IP6_EXTHDR_GET(dstopts, struct ip6_dest *, m, off, dstoptlen);
	if (dstopts == NULL)
		return IPPROTO_DONE;
#endif
	off += dstoptlen;
	dstoptlen -= sizeof(struct ip6_dest);
	opt = (u_int8_t *)dstopts + sizeof(struct ip6_dest);

	/* search header for all options. */
	for (optlen = 0; dstoptlen > 0; dstoptlen -= optlen, opt += optlen) {
		if (*opt != IP6OPT_PAD1 &&
		    (dstoptlen < IP6OPT_MINLEN || *(opt + 1) + 2 > dstoptlen)) {
			IP6STAT_INC(ip6s_toosmall);
			goto bad;
		}

		switch (*opt) {
		case IP6OPT_PAD1:
			optlen = 1;
			break;
		case IP6OPT_PADN:
			optlen = *(opt + 1) + 2;
			break;
		default:		/* unknown option */
			optlen = ip6_unknown_opt(opt, m,
			    opt - mtod(m, u_int8_t *));
			if (optlen == -1)
				return (IPPROTO_DONE);
			optlen += 2;
			break;
		}
	}

	*offp = off;
	return (dstopts->ip6d_nxt);

  bad:
	m_freem(m);
	return (IPPROTO_DONE);
}