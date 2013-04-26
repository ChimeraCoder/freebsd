
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
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include <netgraph/ng_message.h>
#include <netgraph/netgraph.h>
#include <netgraph/ng_atmllc.h>

#include <net/if.h>
#include <net/ethernet.h>	/* for M_HASFCS and ETHER_HDR_LEN */
#include <net/if_atm.h>		/* for struct atmllc */

#define	NG_ATMLLC_HEADER		"\252\252\3\0\200\302"
#define	NG_ATMLLC_HEADER_LEN		(sizeof(struct atmllc))
#define	NG_ATMLLC_TYPE_ETHERNET_FCS	0x0001
#define	NG_ATMLLC_TYPE_FDDI_FCS		0x0004
#define	NG_ATMLLC_TYPE_ETHERNET_NOFCS	0x0007
#define	NG_ATMLLC_TYPE_FDDI_NOFCS	0x000A

struct ng_atmllc_priv {
	hook_p		atm;
	hook_p		ether;
	hook_p		fddi;
};

/* Netgraph methods. */
static ng_constructor_t		ng_atmllc_constructor;
static ng_shutdown_t		ng_atmllc_shutdown;
static ng_rcvmsg_t		ng_atmllc_rcvmsg;
static ng_newhook_t		ng_atmllc_newhook;
static ng_rcvdata_t		ng_atmllc_rcvdata;
static ng_disconnect_t		ng_atmllc_disconnect;

static struct ng_type ng_atmllc_typestruct = {
	.version =	NG_ABI_VERSION,	
	.name =		NG_ATMLLC_NODE_TYPE,
	.constructor =	ng_atmllc_constructor,
	.rcvmsg =	ng_atmllc_rcvmsg,
	.shutdown =	ng_atmllc_shutdown,
	.newhook =	ng_atmllc_newhook,
	.rcvdata =	ng_atmllc_rcvdata,
	.disconnect =	ng_atmllc_disconnect,
};
NETGRAPH_INIT(atmllc, &ng_atmllc_typestruct);

static int
ng_atmllc_constructor(node_p node)
{
	struct	ng_atmllc_priv *priv;

	priv = malloc(sizeof(*priv), M_NETGRAPH, M_WAITOK | M_ZERO);
	NG_NODE_SET_PRIVATE(node, priv);

	return (0);
}

static int
ng_atmllc_rcvmsg(node_p node, item_p item, hook_p lasthook)
{
	struct	ng_mesg *msg;
	int	error;

	error = 0;
	NGI_GET_MSG(item, msg);
	msg->header.flags |= NGF_RESP;
	NG_RESPOND_MSG(error, node, item, msg);
	return (error);
}

static int
ng_atmllc_shutdown(node_p node)
{
	struct	ng_atmllc_priv *priv;

	priv = NG_NODE_PRIVATE(node);

	free(priv, M_NETGRAPH);

	NG_NODE_UNREF(node);

	return (0);
}

static int
ng_atmllc_newhook(node_p node, hook_p hook, const char *name)
{
	struct	ng_atmllc_priv *priv;

	priv = NG_NODE_PRIVATE(node);

	if (strcmp(name, NG_ATMLLC_HOOK_ATM) == 0) {
		if (priv->atm != NULL) {
			return (EISCONN);
		}
		priv->atm = hook;
	} else if (strcmp(name, NG_ATMLLC_HOOK_ETHER) == 0) {
		if (priv->ether != NULL) {
			return (EISCONN);
		}
		priv->ether = hook;
	} else if (strcmp(name, NG_ATMLLC_HOOK_FDDI) == 0) {
		if (priv->fddi != NULL) {
			return (EISCONN);
		}
		priv->fddi = hook;
	} else {
		return (EINVAL);
	}

	return (0);
}

static int
ng_atmllc_rcvdata(hook_p hook, item_p item)
{
	struct	ng_atmllc_priv *priv;
	struct	mbuf *m;
	struct	atmllc *hdr;
	hook_p	outhook;
	u_int	padding;
	int	error;

	priv = NG_NODE_PRIVATE(NG_HOOK_NODE(hook));
	NGI_GET_M(item, m);
	outhook = NULL;
	padding = 0;

	if (hook == priv->atm) {
		/* Ditch the psuedoheader. */
		hdr = mtod(m, struct atmllc *);
		/* m_adj(m, sizeof(struct atm_pseudohdr)); */

		/*
		 * Make sure we have the LLC and ethernet headers.
		 * The ethernet header size is slightly larger than the FDDI
		 * header, which is convenient.
		 */
		if (m->m_len < sizeof(struct atmllc) + ETHER_HDR_LEN) {
			m = m_pullup(m, sizeof(struct atmllc) + ETHER_HDR_LEN);
			if (m == NULL) {
				NG_FREE_ITEM(item);
				return (ENOMEM);
			}
		}

		/* Decode the LLC header. */
		hdr = mtod(m, struct atmllc *);
		if (ATM_LLC_TYPE(hdr) == NG_ATMLLC_TYPE_ETHERNET_NOFCS) {
			m->m_flags &= ~M_HASFCS;
			outhook = priv->ether;
			padding = 2;
		} else if (ATM_LLC_TYPE(hdr) == NG_ATMLLC_TYPE_ETHERNET_FCS) {
			m->m_flags |= M_HASFCS;
			outhook = priv->ether;
			padding = 2;
		} else if (ATM_LLC_TYPE(hdr) == NG_ATMLLC_TYPE_FDDI_NOFCS) {
			m->m_flags &= ~M_HASFCS;
			outhook = priv->fddi;
			padding = 3;
		} else if (ATM_LLC_TYPE(hdr) == NG_ATMLLC_TYPE_FDDI_FCS) {
			m->m_flags |= M_HASFCS;
			outhook = priv->fddi;
			padding = 3;
		} else {
			printf("ng_atmllc: unknown type: %x\n",
			    ATM_LLC_TYPE(hdr));
		}

		/* Remove the LLC header and any padding*/
		m_adj(m, sizeof(struct atmllc) + padding);
	} else if (hook == priv->ether) {
		/* Add the LLC header */
		M_PREPEND(m, NG_ATMLLC_HEADER_LEN + 2, M_NOWAIT);
		if (m == NULL) {
			printf("ng_atmllc: M_PREPEND failed\n");
			NG_FREE_ITEM(item);
			return (ENOMEM);
		}
		hdr = mtod(m, struct atmllc *);
		bzero((void *)hdr, sizeof(struct atmllc) + 2);
		bcopy(NG_ATMLLC_HEADER, hdr->llchdr, 6);
		if ((m->m_flags & M_HASFCS) != 0) {
			ATM_LLC_SETTYPE(hdr, NG_ATMLLC_TYPE_ETHERNET_FCS);
		} else {
			ATM_LLC_SETTYPE(hdr, NG_ATMLLC_TYPE_ETHERNET_NOFCS);
		}
		outhook = priv->atm;
	} else if (hook == priv->fddi) {
		/* Add the LLC header */
		M_PREPEND(m, NG_ATMLLC_HEADER_LEN + 3, M_NOWAIT);
		if (m == NULL) {
			printf("ng_atmllc: M_PREPEND failed\n");
			NG_FREE_ITEM(item);
			return (ENOMEM);
		}
		hdr = mtod(m, struct atmllc *);
		bzero((void *)hdr, sizeof(struct atmllc) + 3);
		bcopy(NG_ATMLLC_HEADER, hdr->llchdr, 6);
		if ((m->m_flags & M_HASFCS) != 0) {
			ATM_LLC_SETTYPE(hdr, NG_ATMLLC_TYPE_FDDI_FCS);
		} else {
			ATM_LLC_SETTYPE(hdr, NG_ATMLLC_TYPE_FDDI_NOFCS);
		}
		outhook = priv->atm;
	}

	if (outhook == NULL) {
		NG_FREE_M(m);
		NG_FREE_ITEM(item);
		return (0);
	}

	NG_FWD_NEW_DATA(error, item, outhook, m);
	return (error);
}

static int
ng_atmllc_disconnect(hook_p hook)
{
	node_p	node;
	struct	ng_atmllc_priv *priv;

	node = NG_HOOK_NODE(hook);
	priv = NG_NODE_PRIVATE(node);

	if (hook == priv->atm) {
		priv->atm = NULL;
	} else if (hook == priv->ether) {
		priv->ether = NULL;
	} else if (hook == priv->fddi) {
		priv->fddi = NULL;
	}

	if (NG_NODE_NUMHOOKS(node) == 0 && NG_NODE_IS_VALID(node)) {
		ng_rmnode_self(node);
	}

	return (0);
}