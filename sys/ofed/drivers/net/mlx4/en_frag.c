
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

#include "opt_inet.h"
#include "mlx4_en.h"

#ifdef INET

#include <net/ethernet.h>
#include <netinet/ip.h>
#include <machine/in_cksum.h>

static struct mlx4_en_ipfrag *find_session(struct mlx4_en_rx_ring *ring,
					   struct ip *iph)
{
	struct mlx4_en_ipfrag *session;
	int i;

	for (i = 0; i < MLX4_EN_NUM_IPFRAG_SESSIONS; i++) {
		session = &ring->ipfrag[i];
		if (session->fragments == NULL)
			continue;
		if (session->daddr == iph->ip_dst.s_addr &&
		    session->saddr == iph->ip_src.s_addr &&
		    session->id == iph->ip_id &&
		    session->protocol == iph->ip_p) {
			return session;
		}
	}
	return NULL;
}

static struct mlx4_en_ipfrag *start_session(struct mlx4_en_rx_ring *ring,
					    struct ip *iph)
{
	struct mlx4_en_ipfrag *session;
	int index = -1;
	int i;

	for (i = 0; i < MLX4_EN_NUM_IPFRAG_SESSIONS; i++) {
		if (ring->ipfrag[i].fragments == NULL) {
			index = i;
			break;
		}
	}
	if (index < 0)
		return NULL;

	session = &ring->ipfrag[index];

	return session;
}


static void flush_session(struct mlx4_en_priv *priv,
			  struct mlx4_en_ipfrag *session,
			  u16 more)
{
	struct mbuf *mb = session->fragments;
	struct ip *iph = mb->m_pkthdr.header;
	struct net_device *dev = mb->m_pkthdr.rcvif;

	/* Update IP length and checksum */
	iph->ip_len = htons(session->total_len);
	iph->ip_off = htons(more | (session->offset >> 3));
	iph->ip_sum = 0;
	iph->ip_sum = in_cksum_skip(mb, iph->ip_hl * 4,
	    (char *)iph - mb->m_data);

	dev->if_input(dev, mb);
	session->fragments = NULL;
	session->last = NULL;
}


static inline void frag_append(struct mlx4_en_priv *priv,
			       struct mlx4_en_ipfrag *session,
			       struct mbuf *mb,
			       unsigned int data_len)
{
	struct mbuf *parent = session->fragments;

	/* Update mb bookkeeping */
	parent->m_pkthdr.len += data_len;
	session->total_len += data_len;

	m_adj(mb, mb->m_pkthdr.len - data_len);

	session->last->m_next = mb;
	for (; mb->m_next != NULL; mb = mb->m_next);
	session->last = mb;
}

int mlx4_en_rx_frags(struct mlx4_en_priv *priv, struct mlx4_en_rx_ring *ring,
		     struct mbuf *mb, struct mlx4_cqe *cqe)
{
	struct mlx4_en_ipfrag *session;
	struct ip *iph;
	u16 ip_len;
	u16 ip_hlen;
	int data_len;
	u16 offset;

	iph = (struct ip *)(mtod(mb, char *) + ETHER_HDR_LEN);
	mb->m_pkthdr.header = iph;
	ip_len = ntohs(iph->ip_len);
	ip_hlen = iph->ip_hl * 4;
	data_len = ip_len - ip_hlen;
	offset = ntohs(iph->ip_off);
	offset &= IP_OFFMASK;
	offset <<= 3;

	session = find_session(ring, iph);
	if (unlikely(in_cksum_skip(mb, ip_hlen, (char *)iph - mb->m_data))) {
		if (session)
			flush_session(priv, session, IP_MF);
		return -EINVAL;
	}
	if (session) {
		if (unlikely(session->offset + session->total_len !=
		    offset + ip_hlen ||
		    session->total_len + mb->m_pkthdr.len > 65536)) {
			flush_session(priv, session, IP_MF);
			goto new_session;
		}
		frag_append(priv, session, mb, data_len);
	} else {
new_session:
		session = start_session(ring, iph);
		if (unlikely(!session))
			return -ENOSPC;

		session->fragments = mb;
		session->daddr = iph->ip_dst.s_addr;
		session->saddr = iph->ip_src.s_addr;
		session->id = iph->ip_id;
		session->protocol = iph->ip_p;
		session->total_len = ip_len;
		session->offset = offset;
		for (; mb->m_next != NULL; mb = mb->m_next);
		session->last = mb;
	}
	if (!(ntohs(iph->ip_off) & IP_MF))
		flush_session(priv, session, 0);

	return 0;
}


void mlx4_en_flush_frags(struct mlx4_en_priv *priv,
			 struct mlx4_en_rx_ring *ring)
{
	struct mlx4_en_ipfrag *session;
	int i;

	for (i = 0; i < MLX4_EN_NUM_IPFRAG_SESSIONS; i++) {
		session = &ring->ipfrag[i];
		if (session->fragments)
			flush_session(priv, session, IP_MF);
	}
}
#endif