
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
#include <sys/domain.h>
#include <sys/lock.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <net/if.h>
#include <net/raw_cb.h>
#include <net/vnet.h>

/*
 * Routines to manage the raw protocol control blocks.
 *
 * TODO:
 *	hash lookups by protocol family/protocol + address family
 *	take care of unique address problems per AF?
 *	redo address binding to allow wildcards
 */

struct mtx rawcb_mtx;
VNET_DEFINE(struct rawcb_list_head, rawcb_list);

static SYSCTL_NODE(_net, OID_AUTO, raw, CTLFLAG_RW, 0,
    "Raw socket infrastructure");

static u_long	raw_sendspace = RAWSNDQ;
SYSCTL_ULONG(_net_raw, OID_AUTO, sendspace, CTLFLAG_RW, &raw_sendspace, 0,
    "Default raw socket send space");

static u_long	raw_recvspace = RAWRCVQ;
SYSCTL_ULONG(_net_raw, OID_AUTO, recvspace, CTLFLAG_RW, &raw_recvspace, 0,
    "Default raw socket receive space");

/*
 * Allocate a control block and a nominal amount of buffer space for the
 * socket.
 */
int
raw_attach(struct socket *so, int proto)
{
	struct rawcb *rp = sotorawcb(so);
	int error;

	/*
	 * It is assumed that raw_attach is called after space has been
	 * allocated for the rawcb; consumer protocols may simply allocate
	 * type struct rawcb, or a wrapper data structure that begins with a
	 * struct rawcb.
	 */
	KASSERT(rp != NULL, ("raw_attach: rp == NULL"));

	error = soreserve(so, raw_sendspace, raw_recvspace);
	if (error)
		return (error);
	rp->rcb_socket = so;
	rp->rcb_proto.sp_family = so->so_proto->pr_domain->dom_family;
	rp->rcb_proto.sp_protocol = proto;
	mtx_lock(&rawcb_mtx);
	LIST_INSERT_HEAD(&V_rawcb_list, rp, list);
	mtx_unlock(&rawcb_mtx);
	return (0);
}

/*
 * Detach the raw connection block and discard socket resources.
 */
void
raw_detach(struct rawcb *rp)
{
	struct socket *so = rp->rcb_socket;

	KASSERT(so->so_pcb == rp, ("raw_detach: so_pcb != rp"));

	so->so_pcb = NULL;
	mtx_lock(&rawcb_mtx);
	LIST_REMOVE(rp, list);
	mtx_unlock(&rawcb_mtx);
	free((caddr_t)(rp), M_PCB);
}