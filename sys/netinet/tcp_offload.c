
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sockopt.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_offload.h>
#define	TCPOUTFLAGS
#include <netinet/tcp_fsm.h>
#include <netinet/toecore.h>

int registered_toedevs;

/*
 * Provide an opportunity for a TOE driver to offload.
 */
int
tcp_offload_connect(struct socket *so, struct sockaddr *nam)
{
	struct ifnet *ifp;
	struct toedev *tod;
	struct rtentry *rt;
	int error = EOPNOTSUPP;

	INP_WLOCK_ASSERT(sotoinpcb(so));
	KASSERT(nam->sa_family == AF_INET || nam->sa_family == AF_INET6,
	    ("%s: called with sa_family %d", __func__, nam->sa_family));

	if (registered_toedevs == 0)
		return (error);

	rt = rtalloc1(nam, 0, 0);
	if (rt)
		RT_UNLOCK(rt);
	else
		return (EHOSTUNREACH);

	ifp = rt->rt_ifp;

	if (nam->sa_family == AF_INET && !(ifp->if_capenable & IFCAP_TOE4))
		goto done;
	if (nam->sa_family == AF_INET6 && !(ifp->if_capenable & IFCAP_TOE6))
		goto done;

	tod = TOEDEV(ifp);
	if (tod != NULL)
		error = tod->tod_connect(tod, so, rt, nam);
done:
	RTFREE(rt);
	return (error);
}

void
tcp_offload_listen_start(struct tcpcb *tp)
{

	INP_WLOCK_ASSERT(tp->t_inpcb);

	EVENTHANDLER_INVOKE(tcp_offload_listen_start, tp);
}

void
tcp_offload_listen_stop(struct tcpcb *tp)
{

	INP_WLOCK_ASSERT(tp->t_inpcb);

	EVENTHANDLER_INVOKE(tcp_offload_listen_stop, tp);
}

void
tcp_offload_input(struct tcpcb *tp, struct mbuf *m)
{
	struct toedev *tod = tp->tod;

	KASSERT(tod != NULL, ("%s: tp->tod is NULL, tp %p", __func__, tp));
	INP_WLOCK_ASSERT(tp->t_inpcb);

	tod->tod_input(tod, tp, m);
}

int
tcp_offload_output(struct tcpcb *tp)
{
	struct toedev *tod = tp->tod;
	int error, flags;

	KASSERT(tod != NULL, ("%s: tp->tod is NULL, tp %p", __func__, tp));
	INP_WLOCK_ASSERT(tp->t_inpcb);

	flags = tcp_outflags[tp->t_state];

	if (flags & TH_RST) {
		/* XXX: avoid repeated calls like we do for FIN */
		error = tod->tod_send_rst(tod, tp);
	} else if ((flags & TH_FIN || tp->t_flags & TF_NEEDFIN) &&
	    (tp->t_flags & TF_SENTFIN) == 0) {
		error = tod->tod_send_fin(tod, tp);
		if (error == 0)
			tp->t_flags |= TF_SENTFIN;
	} else
		error = tod->tod_output(tod, tp);

	return (error);
}

void
tcp_offload_rcvd(struct tcpcb *tp)
{
	struct toedev *tod = tp->tod;

	KASSERT(tod != NULL, ("%s: tp->tod is NULL, tp %p", __func__, tp));
	INP_WLOCK_ASSERT(tp->t_inpcb);

	tod->tod_rcvd(tod, tp);
}

void
tcp_offload_ctloutput(struct tcpcb *tp, int sopt_dir, int sopt_name)
{
	struct toedev *tod = tp->tod;

	KASSERT(tod != NULL, ("%s: tp->tod is NULL, tp %p", __func__, tp));
	INP_WLOCK_ASSERT(tp->t_inpcb);

	tod->tod_ctloutput(tod, tp, sopt_dir, sopt_name);
}

void
tcp_offload_detach(struct tcpcb *tp)
{
	struct toedev *tod = tp->tod;

	KASSERT(tod != NULL, ("%s: tp->tod is NULL, tp %p", __func__, tp));
	INP_WLOCK_ASSERT(tp->t_inpcb);

	tod->tod_pcb_detach(tod, tp);
}