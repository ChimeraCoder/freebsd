
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/mutex.h>
#include <sys/priv.h>
#include <sys/protosw.h>
#include <sys/signalvar.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sx.h>
#include <sys/systm.h>

#include <net/if.h>
#include <net/raw_cb.h>
#include <net/vnet.h>

MTX_SYSINIT(rawcb_mtx, &rawcb_mtx, "rawcb", MTX_DEF);

/*
 * Initialize raw connection block q.
 */
void
raw_init(void)
{

	LIST_INIT(&V_rawcb_list);
}

/*
 * Raw protocol input routine.  Find the socket associated with the packet(s)
 * and move them over.  If nothing exists for this packet, drop it.
 */
/*
 * Raw protocol interface.
 */
void
raw_input(struct mbuf *m0, struct sockproto *proto, struct sockaddr *src)
{

	return (raw_input_ext(m0, proto, src, NULL));
}

void
raw_input_ext(struct mbuf *m0, struct sockproto *proto, struct sockaddr *src,
    raw_input_cb_fn cb)
{
	struct rawcb *rp;
	struct mbuf *m = m0;
	struct socket *last;

	last = 0;
	mtx_lock(&rawcb_mtx);
	LIST_FOREACH(rp, &V_rawcb_list, list) {
		if (rp->rcb_proto.sp_family != proto->sp_family)
			continue;
		if (rp->rcb_proto.sp_protocol  &&
		    rp->rcb_proto.sp_protocol != proto->sp_protocol)
			continue;
		if (cb != NULL && (*cb)(m, proto, src, rp) != 0)
			continue;
		if (last) {
			struct mbuf *n;
			n = m_copy(m, 0, (int)M_COPYALL);
			if (n) {
				if (sbappendaddr(&last->so_rcv, src,
				    n, (struct mbuf *)0) == 0)
					/* should notify about lost packet */
					m_freem(n);
				else
					sorwakeup(last);
			}
		}
		last = rp->rcb_socket;
	}
	if (last) {
		if (sbappendaddr(&last->so_rcv, src,
		    m, (struct mbuf *)0) == 0)
			m_freem(m);
		else
			sorwakeup(last);
	} else
		m_freem(m);
	mtx_unlock(&rawcb_mtx);
}

/*ARGSUSED*/
void
raw_ctlinput(int cmd, struct sockaddr *arg, void *dummy)
{

	if (cmd < 0 || cmd >= PRC_NCMDS)
		return;
	/* INCOMPLETE */
}

static void
raw_uabort(struct socket *so)
{

	KASSERT(sotorawcb(so) != NULL, ("raw_uabort: rp == NULL"));

	soisdisconnected(so);
}

static void
raw_uclose(struct socket *so)
{

	KASSERT(sotorawcb(so) != NULL, ("raw_uabort: rp == NULL"));

	soisdisconnected(so);
}

/* pru_accept is EOPNOTSUPP */

static int
raw_uattach(struct socket *so, int proto, struct thread *td)
{
	int error;

	/*
	 * Implementors of raw sockets will already have allocated the PCB,
	 * so it must be non-NULL here.
	 */
	KASSERT(sotorawcb(so) != NULL, ("raw_uattach: so_pcb == NULL"));

	if (td != NULL) {
		error = priv_check(td, PRIV_NET_RAW);
		if (error)
			return (error);
	}
	return (raw_attach(so, proto));
}

static int
raw_ubind(struct socket *so, struct sockaddr *nam, struct thread *td)
{

	return (EINVAL);
}

static int
raw_uconnect(struct socket *so, struct sockaddr *nam, struct thread *td)
{

	return (EINVAL);
}

/* pru_connect2 is EOPNOTSUPP */
/* pru_control is EOPNOTSUPP */

static void
raw_udetach(struct socket *so)
{
	struct rawcb *rp = sotorawcb(so);

	KASSERT(rp != NULL, ("raw_udetach: rp == NULL"));

	raw_detach(rp);
}

static int
raw_udisconnect(struct socket *so)
{

	KASSERT(sotorawcb(so) != NULL, ("raw_udisconnect: rp == NULL"));

	return (ENOTCONN);
}

/* pru_listen is EOPNOTSUPP */

static int
raw_upeeraddr(struct socket *so, struct sockaddr **nam)
{

	KASSERT(sotorawcb(so) != NULL, ("raw_upeeraddr: rp == NULL"));

	return (ENOTCONN);
}

/* pru_rcvd is EOPNOTSUPP */
/* pru_rcvoob is EOPNOTSUPP */

static int
raw_usend(struct socket *so, int flags, struct mbuf *m, struct sockaddr *nam,
    struct mbuf *control, struct thread *td)
{

	KASSERT(sotorawcb(so) != NULL, ("raw_usend: rp == NULL"));

	if ((flags & PRUS_OOB) || (control && control->m_len)) {
		/* XXXRW: Should control also be freed here? */
		if (m != NULL)
			m_freem(m);
		return (EOPNOTSUPP);
	}

	/*
	 * For historical (bad?) reasons, we effectively ignore the address
	 * argument to sendto(2).  Perhaps we should return an error instead?
	 */
	return ((*so->so_proto->pr_output)(m, so));
}

/* pru_sense is null */

static int
raw_ushutdown(struct socket *so)
{

	KASSERT(sotorawcb(so) != NULL, ("raw_ushutdown: rp == NULL"));

	socantsendmore(so);
	return (0);
}

static int
raw_usockaddr(struct socket *so, struct sockaddr **nam)
{

	KASSERT(sotorawcb(so) != NULL, ("raw_usockaddr: rp == NULL"));

	return (EINVAL);
}

struct pr_usrreqs raw_usrreqs = {
	.pru_abort =		raw_uabort,
	.pru_attach =		raw_uattach,
	.pru_bind =		raw_ubind,
	.pru_connect =		raw_uconnect,
	.pru_detach =		raw_udetach, 
	.pru_disconnect =	raw_udisconnect,
	.pru_peeraddr =		raw_upeeraddr,
	.pru_send =		raw_usend,
	.pru_shutdown =		raw_ushutdown,
	.pru_sockaddr =		raw_usockaddr,
	.pru_close =		raw_uclose,
};