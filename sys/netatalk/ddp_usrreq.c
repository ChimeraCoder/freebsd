
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
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <net/if.h>
#include <net/route.h>
#include <net/netisr.h>

#include <netatalk/at.h>
#include <netatalk/at_var.h>
#include <netatalk/ddp_var.h>
#include <netatalk/ddp_pcb.h>
#include <netatalk/at_extern.h>

static u_long	ddp_sendspace = DDP_MAXSZ; /* Max ddp size + 1 (ddp_type) */
static u_long	ddp_recvspace = 10 * (587 + sizeof(struct sockaddr_at));

static const struct netisr_handler atalk1_nh = {
	.nh_name = "atalk1",
	.nh_handler = at1intr,
	.nh_proto = NETISR_ATALK1,
	.nh_policy = NETISR_POLICY_SOURCE,
};

static const struct netisr_handler atalk2_nh = {
	.nh_name = "atalk2",
	.nh_handler = at2intr,
	.nh_proto = NETISR_ATALK2,
	.nh_policy = NETISR_POLICY_SOURCE,
};

static const struct netisr_handler aarp_nh = {
	.nh_name = "aarp",
	.nh_handler = aarpintr,
	.nh_proto = NETISR_AARP,
	.nh_policy = NETISR_POLICY_SOURCE,
};

static int
ddp_attach(struct socket *so, int proto, struct thread *td)
{
	int error = 0;
	
	KASSERT(sotoddpcb(so) == NULL, ("ddp_attach: ddp != NULL"));

	/*
	 * Allocate socket buffer space first so that it's present
	 * before first use.
	 */
	error = soreserve(so, ddp_sendspace, ddp_recvspace);
	if (error)
		return (error);

	DDP_LIST_XLOCK();
	error = at_pcballoc(so);
	DDP_LIST_XUNLOCK();
	return (error);
}

static void
ddp_detach(struct socket *so)
{
	struct ddpcb *ddp;
	
	ddp = sotoddpcb(so);
	KASSERT(ddp != NULL, ("ddp_detach: ddp == NULL"));

	DDP_LIST_XLOCK();
	DDP_LOCK(ddp);
	at_pcbdetach(so, ddp);
	DDP_LIST_XUNLOCK();
}

static int      
ddp_bind(struct socket *so, struct sockaddr *nam, struct thread *td)
{
	struct ddpcb *ddp;
	int error = 0;
	
	ddp = sotoddpcb(so);
	KASSERT(ddp != NULL, ("ddp_bind: ddp == NULL"));

	DDP_LIST_XLOCK();
	DDP_LOCK(ddp);
	error = at_pcbsetaddr(ddp, nam, td);
	DDP_UNLOCK(ddp);
	DDP_LIST_XUNLOCK();
	return (error);
}
    
static int
ddp_connect(struct socket *so, struct sockaddr *nam, struct thread *td)
{
	struct ddpcb *ddp;
	int error = 0;
	
	ddp = sotoddpcb(so);
	KASSERT(ddp != NULL, ("ddp_connect: ddp == NULL"));

	DDP_LIST_XLOCK();
	DDP_LOCK(ddp);
	if (ddp->ddp_fsat.sat_port != ATADDR_ANYPORT) {
		DDP_UNLOCK(ddp);
		DDP_LIST_XUNLOCK();
		return (EISCONN);
	}

	error = at_pcbconnect( ddp, nam, td );
	DDP_UNLOCK(ddp);
	DDP_LIST_XUNLOCK();
	if (error == 0)
		soisconnected(so);
	return (error);
}

static int
ddp_disconnect(struct socket *so)
{
	struct ddpcb *ddp;
	
	ddp = sotoddpcb(so);
	KASSERT(ddp != NULL, ("ddp_disconnect: ddp == NULL"));

	DDP_LOCK(ddp);
	if (ddp->ddp_fsat.sat_addr.s_node == ATADDR_ANYNODE) {
		DDP_UNLOCK(ddp);
		return (ENOTCONN);
	}

	at_pcbdisconnect(ddp);
	ddp->ddp_fsat.sat_addr.s_node = ATADDR_ANYNODE;
	DDP_UNLOCK(ddp);
	soisdisconnected(so);
	return (0);
}

static int
ddp_shutdown(struct socket *so)
{

	KASSERT(sotoddpcb(so) != NULL, ("ddp_shutdown: ddp == NULL"));

	socantsendmore(so);
	return (0);
}

static int
ddp_send(struct socket *so, int flags, struct mbuf *m, struct sockaddr *addr,
    struct mbuf *control, struct thread *td)
{
	struct ddpcb *ddp;
	int error = 0;
	
	ddp = sotoddpcb(so);
	KASSERT(ddp != NULL, ("ddp_send: ddp == NULL"));

    	if (control && control->m_len)
		return (EINVAL);

	if (addr != NULL) {
		DDP_LIST_XLOCK();
		DDP_LOCK(ddp);
		if (ddp->ddp_fsat.sat_port != ATADDR_ANYPORT) {
			error = EISCONN;
			goto out;
		}

		error = at_pcbconnect(ddp, addr, td);
		if (error == 0) {
			error = ddp_output(m, so);
			at_pcbdisconnect(ddp);
		}
out:
		DDP_UNLOCK(ddp);
		DDP_LIST_XUNLOCK();
	} else {
		DDP_LOCK(ddp);
		if (ddp->ddp_fsat.sat_port == ATADDR_ANYPORT)
			error = ENOTCONN;
		else
			error = ddp_output(m, so);
		DDP_UNLOCK(ddp);
	}
	return (error);
}

/*
 * XXXRW: This is never called because we only invoke abort on stream
 * protocols.
 */
static void
ddp_abort(struct socket *so)
{
	struct ddpcb	*ddp;
	
	ddp = sotoddpcb(so);
	KASSERT(ddp != NULL, ("ddp_abort: ddp == NULL"));

	DDP_LOCK(ddp);
	at_pcbdisconnect(ddp);
	DDP_UNLOCK(ddp);
	soisdisconnected(so);
}

static void
ddp_close(struct socket *so)
{
	struct ddpcb	*ddp;
	
	ddp = sotoddpcb(so);
	KASSERT(ddp != NULL, ("ddp_close: ddp == NULL"));

	DDP_LOCK(ddp);
	at_pcbdisconnect(ddp);
	DDP_UNLOCK(ddp);
	soisdisconnected(so);
}

void 
ddp_init(void)
{

	DDP_LIST_LOCK_INIT();
	TAILQ_INIT(&at_ifaddrhead);
	netisr_register(&atalk1_nh);
	netisr_register(&atalk2_nh);
	netisr_register(&aarp_nh);
}

#if 0
static void 
ddp_clean(void)
{
	struct ddpcp	*ddp;

	for (ddp = ddpcb_list; ddp != NULL; ddp = ddp->ddp_next)
		at_pcbdetach(ddp->ddp_socket, ddp);
	DDP_LIST_LOCK_DESTROY();
}
#endif

static int
at_getpeeraddr(struct socket *so, struct sockaddr **nam)
{

	return (EOPNOTSUPP);
}

static int
at_getsockaddr(struct socket *so, struct sockaddr **nam)
{
	struct ddpcb	*ddp;

	ddp = sotoddpcb(so);
	KASSERT(ddp != NULL, ("at_getsockaddr: ddp == NULL"));

	DDP_LOCK(ddp);
	at_sockaddr(ddp, nam);
	DDP_UNLOCK(ddp);
	return (0);
}

struct pr_usrreqs ddp_usrreqs = {
	.pru_abort =		ddp_abort,
	.pru_attach =		ddp_attach,
	.pru_bind =		ddp_bind,
	.pru_connect =		ddp_connect,
	.pru_control =		at_control,
	.pru_detach =		ddp_detach,
	.pru_disconnect =	ddp_disconnect,
	.pru_peeraddr =		at_getpeeraddr,
	.pru_send =		ddp_send,
	.pru_shutdown =		ddp_shutdown,
	.pru_sockaddr =		at_getsockaddr,
	.pru_close =		ddp_close,
};