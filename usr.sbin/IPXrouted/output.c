
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
static const char sccsid[] = "@(#)output.c	8.1 (Berkeley) 6/5/93";
#endif /* not lint */

/*
 * Routing Table Management Daemon
 */
#include <unistd.h>
#include "defs.h"

/*
 * Apply the function "f" to all non-passive
 * interfaces.  If the interface supports the
 * use of broadcasting use it, otherwise address
 * the output to the known router.
 */
void
toall(f, except, changesonly)
	void (*f)(struct sockaddr *, int, struct interface *, int);
	struct rt_entry *except;
	int changesonly;
{
	register struct interface *ifp;
	register struct sockaddr *dst;
	register int flags;
	register struct rt_entry *trt;
	int onlist;
	extern struct interface *ifnet;

	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_flags & IFF_PASSIVE)
			continue;

		/*
		 * Don't send it on interfaces in the except list.
		 */
		onlist = 0;
		trt = except;
		while(trt) {
			if (ifp == trt->rt_ifp) {
				onlist = 1;
				break;
			}
			trt = trt->rt_clone;
		}
		if (onlist)
			continue;

		dst = ifp->int_flags & IFF_BROADCAST ? &ifp->int_broadaddr :
		      ifp->int_flags & IFF_POINTOPOINT ? &ifp->int_dstaddr :
		      &ifp->int_addr;
		flags = ifp->int_flags & IFF_INTERFACE ? MSG_DONTROUTE : 0;
		(*f)(dst, flags, ifp, changesonly);
	}
}

/*
 * Output a preformed packet.
 */
void
sndmsg(dst, flags, ifp, changesonly)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
	int changesonly;
{

	(*afswitch[dst->sa_family].af_output)
		(ripsock, flags, dst, sizeof (struct rip));
	TRACE_OUTPUT(ifp, dst, sizeof (struct rip));
}

/*
 * Supply dst with the contents of the routing tables.
 * If this won't fit in one packet, chop it up into several.
 *
 * This must be done using the split horizon algorithm.
 * 1. Don't send routing info to the interface from where it was received.
 * 2. Don't publish an interface to itself.
 * 3. If a route is received from more than one interface and the cost is
 *    the same, don't publish it on either interface. I am calling this
 *    clones.
 */
void
supply(dst, flags, ifp, changesonly)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
	int changesonly;
{
	register struct rt_entry *rt;
	register struct rt_entry *crt; /* Clone route */
	register struct rthash *rh;
	register struct netinfo *nn;
	register struct netinfo *n = msg->rip_nets;
	struct sockaddr_ipx *sipx =  (struct sockaddr_ipx *) dst;
	af_output_t *output = afswitch[dst->sa_family].af_output;
	int size, metric, ticks;
	union ipx_net net;
	int delay = 0;

	if (sipx->sipx_port == 0)
		sipx->sipx_port = htons(IPXPORT_RIP);

	msg->rip_cmd = ntohs(RIPCMD_RESPONSE);
	for (rh = nethash; rh < &nethash[ROUTEHASHSIZ]; rh++)
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		size = (char *)n - (char *)msg;
		if (size >= ((MAXRIPNETS * sizeof (struct netinfo)) +
				sizeof (msg->rip_cmd))) {
			(*output)(ripsock, flags, dst, size);
			TRACE_OUTPUT(ifp, dst, size);
			n = msg->rip_nets;
			delay++; 
			if(delay == 2) {
				usleep(50000);
				delay = 0;
			}
		}

		if (changesonly && !(rt->rt_state & RTS_CHANGED))
			continue;

		/*
		 * This should do rule one and two of the split horizon
		 * algorithm.
		 */
		if (rt->rt_ifp == ifp)
			continue;

		/*
		 * Rule 3.
		 * Look if we have clones (different routes to the same
		 * place with exactly the same cost).
		 *
		 * We should not publish on any of the clone interfaces.
		 */
		crt = rt->rt_clone;
		while (crt) {
			if (crt->rt_ifp == ifp)
				goto next;
			crt = crt->rt_clone;
		}

		sipx = (struct sockaddr_ipx *)&rt->rt_dst;
	        if ((rt->rt_flags & (RTF_HOST|RTF_GATEWAY)) == RTF_HOST)
			sipx = (struct sockaddr_ipx *)&rt->rt_router;
		if (rt->rt_metric == HOPCNT_INFINITY)
			metric = HOPCNT_INFINITY;
		else {
			metric = rt->rt_metric + 1;
			/*
			 * We don't advertize routes with more than 15 hops.
			 */
			if (metric >= HOPCNT_INFINITY)
				continue;
		}
		/* XXX One day we should cater for slow interfaces also. */
		ticks = rt->rt_ticks + 1;
		net = sipx->sipx_addr.x_net;

		/*
		 * Make sure that we don't put out a two net entries
		 * for a pt to pt link (one for the G route, one for the if)
		 * This is a kludge, and won't work if there are lots of nets.
		 */
		for (nn = msg->rip_nets; nn < n; nn++) {
			if (ipx_neteqnn(net, nn->rip_dst)) {
				if (ticks < ntohs(nn->rip_ticks)) {
					nn->rip_metric = htons(metric);
					nn->rip_ticks = htons(ticks);
				} else if ((ticks == ntohs(nn->rip_ticks)) &&
					   (metric < ntohs(nn->rip_metric))) {
					nn->rip_metric = htons(metric);
					nn->rip_ticks = htons(ticks);
				}
				goto next;
			}
		}
		n->rip_dst = net;
		n->rip_metric = htons(metric);
		n->rip_ticks = htons(ticks);
		n++;
	next:;
	}
	if (n != msg->rip_nets) {
		size = (char *)n - (char *)msg;
		(*output)(ripsock, flags, dst, size);
		TRACE_OUTPUT(ifp, dst, size);
	}
}