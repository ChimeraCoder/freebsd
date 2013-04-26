
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

#include "opt_mac.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/sbuf.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/file.h>
#include <sys/namei.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/if_var.h>

#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

static struct label *
mac_ip6q_label_alloc(int flag)
{
	struct label *label;
	int error;

	label = mac_labelzone_alloc(flag);
	if (label == NULL)
		return (NULL);

	if (flag & M_WAITOK)
		MAC_POLICY_CHECK(ip6q_init_label, label, flag);
	else
		MAC_POLICY_CHECK_NOSLEEP(ip6q_init_label, label, flag);
	if (error) {
		MAC_POLICY_PERFORM_NOSLEEP(ip6q_destroy_label, label);
		mac_labelzone_free(label);
		return (NULL);
	}
	return (label);
}

int
mac_ip6q_init(struct ip6q *q6, int flag)
{

	if (mac_labeled & MPC_OBJECT_IP6Q) {
		q6->ip6q_label = mac_ip6q_label_alloc(flag);
		if (q6->ip6q_label == NULL)
			return (ENOMEM);
	} else
		q6->ip6q_label = NULL;
	return (0);
}

static void
mac_ip6q_label_free(struct label *label)
{

	MAC_POLICY_PERFORM_NOSLEEP(ip6q_destroy_label, label);
	mac_labelzone_free(label);
}

void
mac_ip6q_destroy(struct ip6q *q6)
{

	if (q6->ip6q_label != NULL) {
		mac_ip6q_label_free(q6->ip6q_label);
		q6->ip6q_label = NULL;
	}
}

void
mac_ip6q_reassemble(struct ip6q *q6, struct mbuf *m)
{
	struct label *label;

	if (mac_policy_count == 0)
		return;

	label = mac_mbuf_to_label(m);

	MAC_POLICY_PERFORM_NOSLEEP(ip6q_reassemble, q6, q6->ip6q_label, m,
	    label);
}

void
mac_ip6q_create(struct mbuf *m, struct ip6q *q6)
{
	struct label *label;

	if (mac_policy_count == 0)
		return;

	label = mac_mbuf_to_label(m);

	MAC_POLICY_PERFORM_NOSLEEP(ip6q_create, m, label, q6,
	    q6->ip6q_label);
}

int
mac_ip6q_match(struct mbuf *m, struct ip6q *q6)
{
	struct label *label;
	int result;

	if (mac_policy_count == 0)
		return (1);

	label = mac_mbuf_to_label(m);

	result = 1;
	MAC_POLICY_BOOLEAN_NOSLEEP(ip6q_match, &&, m, label, q6,
	    q6->ip6q_label);

	return (result);
}

void
mac_ip6q_update(struct mbuf *m, struct ip6q *q6)
{
	struct label *label;

	if (mac_policy_count == 0)
		return;

	label = mac_mbuf_to_label(m);

	MAC_POLICY_PERFORM_NOSLEEP(ip6q_update, m, label, q6,
	    q6->ip6q_label);
}

void
mac_netinet6_nd6_send(struct ifnet *ifp, struct mbuf *m)
{
	struct label *mlabel;

	if (mac_policy_count == 0)
		return;

	mlabel = mac_mbuf_to_label(m);

	MAC_POLICY_PERFORM_NOSLEEP(netinet6_nd6_send, ifp, ifp->if_label, m,
	    mlabel);
}