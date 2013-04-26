
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-sunrpc.c,v 1.47 2005-04-27 21:43:48 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * At least on HP-UX:
 *
 *	1) getrpcbynumber() is declared in <netdb.h>, not any of the RPC
 *	   header files
 *
 * and
 *
 *	2) if _XOPEN_SOURCE_EXTENDED is defined, <netdb.h> doesn't declare
 *	   it
 *
 * so we undefine it.
 */
#undef _XOPEN_SOURCE_EXTENDED

#include <tcpdump-stdinc.h>

#if defined(HAVE_GETRPCBYNUMBER) && defined(HAVE_RPC_RPC_H)
#include <rpc/rpc.h>
#ifdef HAVE_RPC_RPCENT_H
#include <rpc/rpcent.h>
#endif /* HAVE_RPC_RPCENT_H */
#endif /* defined(HAVE_GETRPCBYNUMBER) && defined(HAVE_RPC_RPC_H) */

#include <stdio.h>
#include <string.h>

#include "interface.h"
#include "addrtoname.h"
#include "extract.h"

#include "ip.h"
#ifdef INET6
#include "ip6.h"
#endif

#include "rpc_auth.h"
#include "rpc_msg.h"
#include "pmap_prot.h"

static struct tok proc2str[] = {
	{ SUNRPC_PMAPPROC_NULL,		"null" },
	{ SUNRPC_PMAPPROC_SET,		"set" },
	{ SUNRPC_PMAPPROC_UNSET,	"unset" },
	{ SUNRPC_PMAPPROC_GETPORT,	"getport" },
	{ SUNRPC_PMAPPROC_DUMP,		"dump" },
	{ SUNRPC_PMAPPROC_CALLIT,	"call" },
	{ 0,				NULL }
};

/* Forwards */
static char *progstr(u_int32_t);

void
sunrpcrequest_print(register const u_char *bp, register u_int length,
		    register const u_char *bp2)
{
	register const struct sunrpc_msg *rp;
	register const struct ip *ip;
#ifdef INET6
	register const struct ip6_hdr *ip6;
#endif
	u_int32_t x;
	char srcid[20], dstid[20];	/*fits 32bit*/

	rp = (struct sunrpc_msg *)bp;

	if (!nflag) {
		snprintf(srcid, sizeof(srcid), "0x%x",
		    EXTRACT_32BITS(&rp->rm_xid));
		strlcpy(dstid, "sunrpc", sizeof(dstid));
	} else {
		snprintf(srcid, sizeof(srcid), "0x%x",
		    EXTRACT_32BITS(&rp->rm_xid));
		snprintf(dstid, sizeof(dstid), "0x%x", SUNRPC_PMAPPORT);
	}

	switch (IP_V((struct ip *)bp2)) {
	case 4:
		ip = (struct ip *)bp2;
		printf("%s.%s > %s.%s: %d",
		    ipaddr_string(&ip->ip_src), srcid,
		    ipaddr_string(&ip->ip_dst), dstid, length);
		break;
#ifdef INET6
	case 6:
		ip6 = (struct ip6_hdr *)bp2;
		printf("%s.%s > %s.%s: %d",
		    ip6addr_string(&ip6->ip6_src), srcid,
		    ip6addr_string(&ip6->ip6_dst), dstid, length);
		break;
#endif
	default:
		printf("%s.%s > %s.%s: %d", "?", srcid, "?", dstid, length);
		break;
	}

	printf(" %s", tok2str(proc2str, " proc #%u",
	    EXTRACT_32BITS(&rp->rm_call.cb_proc)));
	x = EXTRACT_32BITS(&rp->rm_call.cb_rpcvers);
	if (x != 2)
		printf(" [rpcver %u]", x);

	switch (EXTRACT_32BITS(&rp->rm_call.cb_proc)) {

	case SUNRPC_PMAPPROC_SET:
	case SUNRPC_PMAPPROC_UNSET:
	case SUNRPC_PMAPPROC_GETPORT:
	case SUNRPC_PMAPPROC_CALLIT:
		x = EXTRACT_32BITS(&rp->rm_call.cb_prog);
		if (!nflag)
			printf(" %s", progstr(x));
		else
			printf(" %u", x);
		printf(".%u", EXTRACT_32BITS(&rp->rm_call.cb_vers));
		break;
	}
}

static char *
progstr(prog)
	u_int32_t prog;
{
#if defined(HAVE_GETRPCBYNUMBER) && defined(HAVE_RPC_RPC_H)
	register struct rpcent *rp;
#endif
	static char buf[32];
	static u_int32_t lastprog = 0;

	if (lastprog != 0 && prog == lastprog)
		return (buf);
#if defined(HAVE_GETRPCBYNUMBER) && defined(HAVE_RPC_RPC_H)
	rp = getrpcbynumber(prog);
	if (rp == NULL)
#endif
		(void) snprintf(buf, sizeof(buf), "#%u", prog);
#if defined(HAVE_GETRPCBYNUMBER) && defined(HAVE_RPC_RPC_H)
	else
		strlcpy(buf, rp->r_name, sizeof(buf));
#endif
	return (buf);
}