
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

#include <netinet/sctp_os.h>
#include <netinet/sctp_pcb.h>
#include <netinet/sctputil.h>
#include <netinet/sctp_var.h>
#include <netinet/sctp_var.h>
#include <netinet/sctp_sysctl.h>
#include <netinet/sctp.h>
#include <netinet/sctp_uio.h>
#include <netinet/sctp_peeloff.h>
#include <netinet/sctputil.h>
#include <netinet/sctp_auth.h>


int
sctp_can_peel_off(struct socket *head, sctp_assoc_t assoc_id)
{
	struct sctp_inpcb *inp;
	struct sctp_tcb *stcb;
	uint32_t state;

	if (head == NULL) {
		SCTP_LTRACE_ERR_RET(NULL, NULL, NULL, SCTP_FROM_SCTP_PEELOFF, EBADF);
		return (EBADF);
	}
	inp = (struct sctp_inpcb *)head->so_pcb;
	if (inp == NULL) {
		SCTP_LTRACE_ERR_RET(NULL, NULL, NULL, SCTP_FROM_SCTP_PEELOFF, EFAULT);
		return (EFAULT);
	}
	if ((inp->sctp_flags & SCTP_PCB_FLAGS_TCPTYPE) ||
	    (inp->sctp_flags & SCTP_PCB_FLAGS_IN_TCPPOOL)) {
		SCTP_LTRACE_ERR_RET(inp, NULL, NULL, SCTP_FROM_SCTP_PEELOFF, EOPNOTSUPP);
		return (EOPNOTSUPP);
	}
	stcb = sctp_findassociation_ep_asocid(inp, assoc_id, 1);
	if (stcb == NULL) {
		SCTP_LTRACE_ERR_RET(inp, stcb, NULL, SCTP_FROM_SCTP_PEELOFF, ENOENT);
		return (ENOENT);
	}
	state = SCTP_GET_STATE((&stcb->asoc));
	if ((state == SCTP_STATE_EMPTY) ||
	    (state == SCTP_STATE_INUSE)) {
		SCTP_TCB_UNLOCK(stcb);
		SCTP_LTRACE_ERR_RET(inp, stcb, NULL, SCTP_FROM_SCTP_PEELOFF, ENOTCONN);
		return (ENOTCONN);
	}
	SCTP_TCB_UNLOCK(stcb);
	/* We are clear to peel this one off */
	return (0);
}

int
sctp_do_peeloff(struct socket *head, struct socket *so, sctp_assoc_t assoc_id)
{
	struct sctp_inpcb *inp, *n_inp;
	struct sctp_tcb *stcb;
	uint32_t state;

	inp = (struct sctp_inpcb *)head->so_pcb;
	if (inp == NULL) {
		SCTP_LTRACE_ERR_RET(inp, NULL, NULL, SCTP_FROM_SCTP_PEELOFF, EFAULT);
		return (EFAULT);
	}
	stcb = sctp_findassociation_ep_asocid(inp, assoc_id, 1);
	if (stcb == NULL) {
		SCTP_LTRACE_ERR_RET(inp, NULL, NULL, SCTP_FROM_SCTP_PEELOFF, ENOTCONN);
		return (ENOTCONN);
	}
	state = SCTP_GET_STATE((&stcb->asoc));
	if ((state == SCTP_STATE_EMPTY) ||
	    (state == SCTP_STATE_INUSE)) {
		SCTP_TCB_UNLOCK(stcb);
		SCTP_LTRACE_ERR_RET(inp, NULL, NULL, SCTP_FROM_SCTP_PEELOFF, ENOTCONN);
		return (ENOTCONN);
	}
	n_inp = (struct sctp_inpcb *)so->so_pcb;
	n_inp->sctp_flags = (SCTP_PCB_FLAGS_UDPTYPE |
	    SCTP_PCB_FLAGS_CONNECTED |
	    SCTP_PCB_FLAGS_IN_TCPPOOL |	/* Turn on Blocking IO */
	    (SCTP_PCB_COPY_FLAGS & inp->sctp_flags));
	n_inp->sctp_socket = so;
	n_inp->sctp_features = inp->sctp_features;
	n_inp->sctp_mobility_features = inp->sctp_mobility_features;
	n_inp->sctp_frag_point = inp->sctp_frag_point;
	n_inp->sctp_cmt_on_off = inp->sctp_cmt_on_off;
	n_inp->sctp_ecn_enable = inp->sctp_ecn_enable;
	n_inp->partial_delivery_point = inp->partial_delivery_point;
	n_inp->sctp_context = inp->sctp_context;
	n_inp->local_strreset_support = inp->local_strreset_support;
	n_inp->inp_starting_point_for_iterator = NULL;
	/* copy in the authentication parameters from the original endpoint */
	if (n_inp->sctp_ep.local_hmacs)
		sctp_free_hmaclist(n_inp->sctp_ep.local_hmacs);
	n_inp->sctp_ep.local_hmacs =
	    sctp_copy_hmaclist(inp->sctp_ep.local_hmacs);
	if (n_inp->sctp_ep.local_auth_chunks)
		sctp_free_chunklist(n_inp->sctp_ep.local_auth_chunks);
	n_inp->sctp_ep.local_auth_chunks =
	    sctp_copy_chunklist(inp->sctp_ep.local_auth_chunks);
	(void)sctp_copy_skeylist(&inp->sctp_ep.shared_keys,
	    &n_inp->sctp_ep.shared_keys);
	/*
	 * Now we must move it from one hash table to another and get the
	 * stcb in the right place.
	 */
	sctp_move_pcb_and_assoc(inp, n_inp, stcb);
	atomic_add_int(&stcb->asoc.refcnt, 1);
	SCTP_TCB_UNLOCK(stcb);

	sctp_pull_off_control_to_new_inp(inp, n_inp, stcb, SBL_WAIT);
	atomic_subtract_int(&stcb->asoc.refcnt, 1);

	return (0);
}