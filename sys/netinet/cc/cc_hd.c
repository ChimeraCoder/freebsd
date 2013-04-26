
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

/*
 * An implementation of the Hamilton Institute's delay-based congestion control
 * algorithm for FreeBSD, based on "A strategy for fair coexistence of loss and
 * delay-based congestion control algorithms," by L. Budzisz, R. Stanojevic, R.
 * Shorten, and F. Baker, IEEE Commun. Lett., vol. 13, no. 7, pp. 555--557, Jul.
 * 2009.
 *
 * Originally released as part of the NewTCP research project at Swinburne
 * University of Technology's Centre for Advanced Internet Architectures,
 * Melbourne, Australia, which was made possible in part by a grant from the
 * Cisco University Research Program Fund at Community Foundation Silicon
 * Valley. More details are available at:
 *   http://caia.swin.edu.au/urp/newtcp/
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/khelp.h>
#include <sys/limits.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <net/if.h>
#include <net/vnet.h>

#include <netinet/cc.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

#include <netinet/cc/cc_module.h>

#include <netinet/khelp/h_ertt.h>

#define	CAST_PTR_INT(X)	(*((int*)(X)))

/* Largest possible number returned by random(). */
#define	RANDOM_MAX	INT_MAX

static void	hd_ack_received(struct cc_var *ccv, uint16_t ack_type);
static int	hd_mod_init(void);

static int ertt_id;

static VNET_DEFINE(uint32_t, hd_qthresh) = 20;
static VNET_DEFINE(uint32_t, hd_qmin) = 5;
static VNET_DEFINE(uint32_t, hd_pmax) = 5;
#define	V_hd_qthresh	VNET(hd_qthresh)
#define	V_hd_qmin	VNET(hd_qmin)
#define	V_hd_pmax	VNET(hd_pmax)

struct cc_algo hd_cc_algo = {
	.name = "hd",
	.ack_received = hd_ack_received,
	.mod_init = hd_mod_init
};

/*
 * Hamilton backoff function. Returns 1 if we should backoff or 0 otherwise.
 */
static __inline int
should_backoff(int qdly, int maxqdly)
{
	unsigned long p;

	if (qdly < V_hd_qthresh) {
		p = (((RANDOM_MAX / 100) * V_hd_pmax) /
		    (V_hd_qthresh - V_hd_qmin)) * (qdly - V_hd_qmin);
	} else {
		if (qdly > V_hd_qthresh)
			p = (((RANDOM_MAX / 100) * V_hd_pmax) /
			    (maxqdly - V_hd_qthresh)) * (maxqdly - qdly);
		else
			p = (RANDOM_MAX / 100) * V_hd_pmax;
	}

	return (random() < p);
}

/*
 * If the ack type is CC_ACK, and the inferred queueing delay is greater than
 * the Qmin threshold, cwnd is reduced probabilistically. When backing off due
 * to delay, HD behaves like NewReno when an ECN signal is received. HD behaves
 * as NewReno in all other circumstances.
 */
static void
hd_ack_received(struct cc_var *ccv, uint16_t ack_type)
{
	struct ertt *e_t;
	int qdly;

	if (ack_type == CC_ACK) {
		e_t = khelp_get_osd(CCV(ccv, osd), ertt_id);

		if (e_t->rtt && e_t->minrtt && V_hd_qthresh > 0) {
			qdly = e_t->rtt - e_t->minrtt;

			if (qdly > V_hd_qmin &&
			    !IN_RECOVERY(CCV(ccv, t_flags))) {
				/* Probabilistic backoff of cwnd. */
				if (should_backoff(qdly,
				    e_t->maxrtt - e_t->minrtt)) {
					/*
					 * Update cwnd and ssthresh update to
					 * half cwnd and behave like an ECN (ie
					 * not a packet loss).
					 */
					newreno_cc_algo.cong_signal(ccv,
					    CC_ECN);
					return;
				}
			}
		}
	}
	newreno_cc_algo.ack_received(ccv, ack_type); /* As for NewReno. */
}

static int
hd_mod_init(void)
{

	ertt_id = khelp_get_id("ertt");
	if (ertt_id <= 0) {
		printf("%s: h_ertt module not found\n", __func__);
		return (ENOENT);
	}

	hd_cc_algo.after_idle = newreno_cc_algo.after_idle;
	hd_cc_algo.cong_signal = newreno_cc_algo.cong_signal;
	hd_cc_algo.post_recovery = newreno_cc_algo.post_recovery;

	return (0);
}

static int
hd_pmax_handler(SYSCTL_HANDLER_ARGS)
{
	int error;
	uint32_t new;

	new = V_hd_pmax;
	error = sysctl_handle_int(oidp, &new, 0, req);
	if (error == 0 && req->newptr != NULL) {
		if (CAST_PTR_INT(req->newptr) == 0 ||
		    CAST_PTR_INT(req->newptr) > 100)
			error = EINVAL;
		else
			V_hd_pmax = new;
	}

	return (error);
}

static int
hd_qmin_handler(SYSCTL_HANDLER_ARGS)
{
	int error;
	uint32_t new;

	new = V_hd_qmin;
	error = sysctl_handle_int(oidp, &new, 0, req);
	if (error == 0 && req->newptr != NULL) {
		if (CAST_PTR_INT(req->newptr) > V_hd_qthresh)
			error = EINVAL;
		else
			V_hd_qmin = new;
	}

	return (error);
}

static int
hd_qthresh_handler(SYSCTL_HANDLER_ARGS)
{
	int error;
	uint32_t new;

	new = V_hd_qthresh;
	error = sysctl_handle_int(oidp, &new, 0, req);
	if (error == 0 && req->newptr != NULL) {
		if (CAST_PTR_INT(req->newptr) < 1 ||
		    CAST_PTR_INT(req->newptr) < V_hd_qmin)
			error = EINVAL;
		else
			V_hd_qthresh = new;
	}

	return (error);
}

SYSCTL_DECL(_net_inet_tcp_cc_hd);
SYSCTL_NODE(_net_inet_tcp_cc, OID_AUTO, hd, CTLFLAG_RW, NULL,
    "Hamilton delay-based congestion control related settings");

SYSCTL_VNET_PROC(_net_inet_tcp_cc_hd, OID_AUTO, queue_threshold,
    CTLTYPE_UINT|CTLFLAG_RW, &VNET_NAME(hd_qthresh), 20, &hd_qthresh_handler,
    "IU", "queueing congestion threshold (qth) in ticks");

SYSCTL_VNET_PROC(_net_inet_tcp_cc_hd, OID_AUTO, pmax,
    CTLTYPE_UINT|CTLFLAG_RW, &VNET_NAME(hd_pmax), 5, &hd_pmax_handler,
    "IU", "per packet maximum backoff probability as a percentage");

SYSCTL_VNET_PROC(_net_inet_tcp_cc_hd, OID_AUTO, queue_min,
    CTLTYPE_UINT|CTLFLAG_RW, &VNET_NAME(hd_qmin), 5, &hd_qmin_handler,
    "IU", "minimum queueing delay threshold (qmin) in ticks");

DECLARE_CC_MODULE(hd, &hd_cc_algo);
MODULE_DEPEND(hd, ertt, 1, 1, 1);