
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
 * Developed by the TrustedBSD Project.
 *
 * Limit access to interfaces until they are specifically administratively
 * enabled.  Prevents protocol stack-driven packet leakage in unsafe
 * environments.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <net/bpfdesc.h>
#include <net/if_types.h>

#include <security/mac/mac_policy.h>

SYSCTL_DECL(_security_mac);

static SYSCTL_NODE(_security_mac, OID_AUTO, ifoff, CTLFLAG_RW, 0,
    "TrustedBSD mac_ifoff policy controls");

static int	ifoff_enabled = 1;
SYSCTL_INT(_security_mac_ifoff, OID_AUTO, enabled, CTLFLAG_RW,
    &ifoff_enabled, 0, "Enforce ifoff policy");
TUNABLE_INT("security.mac.ifoff.enabled", &ifoff_enabled);

static int	ifoff_lo_enabled = 1;
SYSCTL_INT(_security_mac_ifoff, OID_AUTO, lo_enabled, CTLFLAG_RW,
    &ifoff_lo_enabled, 0, "Enable loopback interfaces");
TUNABLE_INT("security.mac.ifoff.lo_enabled", &ifoff_lo_enabled);

static int	ifoff_other_enabled = 0;
SYSCTL_INT(_security_mac_ifoff, OID_AUTO, other_enabled, CTLFLAG_RW,
    &ifoff_other_enabled, 0, "Enable other interfaces");
TUNABLE_INT("security.mac.ifoff.other_enabled", &ifoff_other_enabled);

static int	ifoff_bpfrecv_enabled = 0;
SYSCTL_INT(_security_mac_ifoff, OID_AUTO, bpfrecv_enabled, CTLFLAG_RW,
    &ifoff_bpfrecv_enabled, 0, "Enable BPF reception even when interface "
    "is disabled");
TUNABLE_INT("security.mac.ifoff.bpfrecv.enabled", &ifoff_bpfrecv_enabled);

static int
ifnet_check_outgoing(struct ifnet *ifp)
{

	if (!ifoff_enabled)
		return (0);

	if (ifoff_lo_enabled && ifp->if_type == IFT_LOOP)
		return (0);

	if (ifoff_other_enabled && ifp->if_type != IFT_LOOP)
		return (0);

	return (EPERM);
}

static int
ifnet_check_incoming(struct ifnet *ifp, int viabpf)
{
	if (!ifoff_enabled)
		return (0);

	if (ifoff_lo_enabled && ifp->if_type == IFT_LOOP)
		return (0);

	if (ifoff_other_enabled && ifp->if_type != IFT_LOOP)
		return (0);

	if (viabpf && ifoff_bpfrecv_enabled)
		return (0);

	return (EPERM);
}

/*
 * Object-specific entry point implementations are sorted alphabetically by
 * object type and then by operation.
 */
static int
ifoff_bpfdesc_check_receive(struct bpf_d *d, struct label *dlabel,
    struct ifnet *ifp, struct label *ifplabel)
{

	return (ifnet_check_incoming(ifp, 1));
}

static int
ifoff_ifnet_check_transmit(struct ifnet *ifp, struct label *ifplabel,
    struct mbuf *m, struct label *mlabel)
{

	return (ifnet_check_outgoing(ifp));
}

static int
ifoff_inpcb_check_deliver(struct inpcb *inp, struct label *inplabel,
    struct mbuf *m, struct label *mlabel)
{

	M_ASSERTPKTHDR(m);
	if (m->m_pkthdr.rcvif != NULL)
		return (ifnet_check_incoming(m->m_pkthdr.rcvif, 0));

	return (0);
}

static int
ifoff_socket_check_deliver(struct socket *so, struct label *solabel,
    struct mbuf *m, struct label *mlabel)
{

	M_ASSERTPKTHDR(m);
	if (m->m_pkthdr.rcvif != NULL)
		return (ifnet_check_incoming(m->m_pkthdr.rcvif, 0));

	return (0);
}

static struct mac_policy_ops ifoff_ops =
{
	.mpo_bpfdesc_check_receive = ifoff_bpfdesc_check_receive,
	.mpo_ifnet_check_transmit = ifoff_ifnet_check_transmit,
	.mpo_inpcb_check_deliver = ifoff_inpcb_check_deliver,
	.mpo_socket_check_deliver = ifoff_socket_check_deliver,
};

MAC_POLICY_SET(&ifoff_ops, mac_ifoff, "TrustedBSD MAC/ifoff",
    MPC_LOADTIME_FLAG_UNLOADOK, NULL);