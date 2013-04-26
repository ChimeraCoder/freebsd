
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

#include "opt_inet6.h"

#include <sys/param.h>
#include <sys/mbuf.h>

#include <netinet/in.h>
#include <netinet/in_pcb.h>
#ifdef INET6
#include <netinet6/in6_pcb.h>
#endif /* INET6 */

/*
 * Given a hash of whatever the covered tuple might be, return a pcbgroup
 * index.
 */
static __inline u_int
in6_pcbgroup_getbucket(struct inpcbinfo *pcbinfo, uint32_t hash)
{

	return (hash % pcbinfo->ipi_npcbgroups);
}

/*
 * Map a (hashtype, hash) tuple into a connection group, or NULL if the hash 
 * information is insufficient to identify the pcbgroup.
 */
struct inpcbgroup *
in6_pcbgroup_byhash(struct inpcbinfo *pcbinfo, u_int hashtype, uint32_t hash)
{

	return (NULL);
}

struct inpcbgroup *
in6_pcbgroup_bymbuf(struct inpcbinfo *pcbinfo, struct mbuf *m)
{

	return (in6_pcbgroup_byhash(pcbinfo, M_HASHTYPE_GET(m),
	    m->m_pkthdr.flowid));
}

struct inpcbgroup *
in6_pcbgroup_bytuple(struct inpcbinfo *pcbinfo, const struct in6_addr *laddrp,
    u_short lport, const struct in6_addr *faddrp, u_short fport)
{
	uint32_t hash;

	switch (pcbinfo->ipi_hashfields) {
	case IPI_HASHFIELDS_4TUPLE:
		hash = faddrp->s6_addr32[3] ^ fport;
		break;

	case IPI_HASHFIELDS_2TUPLE:
		hash = faddrp->s6_addr32[3] ^ laddrp->s6_addr32[3];
		break;

	default:
		hash = 0;
	}
	return (&pcbinfo->ipi_pcbgroups[in6_pcbgroup_getbucket(pcbinfo,
	    hash)]);
}

struct inpcbgroup *
in6_pcbgroup_byinpcb(struct inpcb *inp)
{

	return (in6_pcbgroup_bytuple(inp->inp_pcbinfo, &inp->in6p_laddr,
	    inp->inp_lport, &inp->in6p_faddr, inp->inp_fport));
}