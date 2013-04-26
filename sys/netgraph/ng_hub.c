
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
#include <sys/errno.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/systm.h>

#include <netgraph/ng_message.h>
#include <netgraph/ng_hub.h>
#include <netgraph/netgraph.h>

#ifdef NG_SEPARATE_MALLOC
static MALLOC_DEFINE(M_NETGRAPH_HUB, "netgraph_hub", "netgraph hub node");
#else
#define M_NETGRAPH_HUB M_NETGRAPH
#endif

/* Per-node private data */
struct ng_hub_private {
	int		persistent;	/* can exist w/o hooks */
};
typedef struct ng_hub_private *priv_p;

/* Netgraph node methods */
static ng_constructor_t	ng_hub_constructor;
static ng_rcvmsg_t	ng_hub_rcvmsg;
static ng_shutdown_t	ng_hub_shutdown;
static ng_rcvdata_t	ng_hub_rcvdata;
static ng_disconnect_t	ng_hub_disconnect;

/* List of commands and how to convert arguments to/from ASCII */
static const struct ng_cmdlist ng_hub_cmdlist[] = {
	{
		NGM_HUB_COOKIE,
		NGM_HUB_SET_PERSISTENT,
		"setpersistent",
		NULL,
		NULL
	},
	{ 0 }
};

static struct ng_type ng_hub_typestruct = {
	.version =	NG_ABI_VERSION,
	.name =		NG_HUB_NODE_TYPE,
	.constructor =	ng_hub_constructor,
	.rcvmsg =	ng_hub_rcvmsg,
	.shutdown =	ng_hub_shutdown,
	.rcvdata =	ng_hub_rcvdata,
	.disconnect =	ng_hub_disconnect,
	.cmdlist =	ng_hub_cmdlist,
};
NETGRAPH_INIT(hub, &ng_hub_typestruct);


static int
ng_hub_constructor(node_p node)
{
	priv_p priv;

	/* Allocate and initialize private info */
	priv = malloc(sizeof(*priv), M_NETGRAPH_HUB, M_WAITOK | M_ZERO);

	NG_NODE_SET_PRIVATE(node, priv);
	return (0);
}

/*
 * Receive a control message
 */
static int
ng_hub_rcvmsg(node_p node, item_p item, hook_p lasthook)
{
	const priv_p priv = NG_NODE_PRIVATE(node);
	int error = 0;
	struct ng_mesg *msg;

	NGI_GET_MSG(item, msg);
	if (msg->header.typecookie == NGM_HUB_COOKIE &&
	    msg->header.cmd == NGM_HUB_SET_PERSISTENT) {
		priv->persistent = 1;
	} else {
		error = EINVAL;
	}

	NG_FREE_MSG(msg);
	return (error);
}

static int
ng_hub_rcvdata(hook_p hook, item_p item)
{
	const node_p node = NG_HOOK_NODE(hook);
	int error = 0;
	hook_p hook2;
	struct mbuf * const m = NGI_M(item), *m2;
	int nhooks;

	if ((nhooks = NG_NODE_NUMHOOKS(node)) == 1) {
		NG_FREE_ITEM(item);
		return (0);
	}
	LIST_FOREACH(hook2, &node->nd_hooks, hk_hooks) {
		if (hook2 == hook)
			continue;
		if (--nhooks == 1)
			NG_FWD_ITEM_HOOK(error, item, hook2);
		else {
			if ((m2 = m_dup(m, M_NOWAIT)) == NULL) {
				NG_FREE_ITEM(item);
				return (ENOBUFS);
			}
			NG_SEND_DATA_ONLY(error, hook2, m2);
			if (error)
				continue;	/* don't give up */
		}
	}

	return (error);
}

/*
 * Shutdown node
 */
static int
ng_hub_shutdown(node_p node)
{
	const priv_p priv = NG_NODE_PRIVATE(node);

	free(priv, M_NETGRAPH_HUB);
	NG_NODE_SET_PRIVATE(node, NULL);
	NG_NODE_UNREF(node);
	return (0);
}

static int
ng_hub_disconnect(hook_p hook)
{
	const priv_p priv = NG_NODE_PRIVATE(NG_HOOK_NODE(hook));

	if (NG_NODE_NUMHOOKS(NG_HOOK_NODE(hook)) == 0 &&
	    NG_NODE_IS_VALID(NG_HOOK_NODE(hook)) && !priv->persistent)
		ng_rmnode_self(NG_HOOK_NODE(hook));
	return (0);
}