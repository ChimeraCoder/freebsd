
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
#ifdef __FreeBSD__
__FBSDID("$FreeBSD$");
#endif

/*
 * IEEE 802.11 send/recv action frame support.
 */

#include "opt_inet.h"
#include "opt_wlan.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h> 
 
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_media.h>
#include <net/ethernet.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_action.h>
#include <net80211/ieee80211_mesh.h>

static int
send_inval(struct ieee80211_node *ni, int cat, int act, void *sa)
{
	return EINVAL;
}

static ieee80211_send_action_func *ba_send_action[8] = {
	send_inval, send_inval, send_inval, send_inval,
	send_inval, send_inval, send_inval, send_inval,
};
static ieee80211_send_action_func *ht_send_action[8] = {
	send_inval, send_inval, send_inval, send_inval,
	send_inval, send_inval, send_inval, send_inval,
};
static ieee80211_send_action_func *meshpl_send_action[8] = {
	send_inval, send_inval, send_inval, send_inval,
	send_inval, send_inval, send_inval, send_inval,
};
static ieee80211_send_action_func *meshaction_send_action[12] = {
	send_inval, send_inval, send_inval, send_inval,
	send_inval, send_inval, send_inval, send_inval,
	send_inval, send_inval, send_inval, send_inval,
};
static ieee80211_send_action_func *vendor_send_action[8] = {
	send_inval, send_inval, send_inval, send_inval,
	send_inval, send_inval, send_inval, send_inval,
};

int
ieee80211_send_action_register(int cat, int act, ieee80211_send_action_func *f)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	switch (cat) {
	case IEEE80211_ACTION_CAT_BA:
		if (act >= N(ba_send_action))
			break;
		ba_send_action[act] = f;
		return 0;
	case IEEE80211_ACTION_CAT_HT:
		if (act >= N(ht_send_action))
			break;
		ht_send_action[act] = f;
		return 0;
	case IEEE80211_ACTION_CAT_SELF_PROT:
		if (act >= N(meshpl_send_action))
			break;
		meshpl_send_action[act] = f;
		return 0;
	case IEEE80211_ACTION_CAT_MESH:
		if (act >= N(meshaction_send_action))
			break;
		meshaction_send_action[act] = f;
		return 0;
		break;
	case IEEE80211_ACTION_CAT_VENDOR:
		if (act >= N(vendor_send_action))
			break;
		vendor_send_action[act] = f;
		return 0;
	}
	return EINVAL;
#undef N
}

void
ieee80211_send_action_unregister(int cat, int act)
{
	ieee80211_send_action_register(cat, act, send_inval);
}

int
ieee80211_send_action(struct ieee80211_node *ni, int cat, int act, void *sa)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	ieee80211_send_action_func *f = send_inval;

	switch (cat) {
	case IEEE80211_ACTION_CAT_BA:
		if (act < N(ba_send_action))
			f = ba_send_action[act];
		break;
	case IEEE80211_ACTION_CAT_HT:
		if (act < N(ht_send_action))
			f = ht_send_action[act];
		break;
	case IEEE80211_ACTION_CAT_SELF_PROT:
		if (act < N(meshpl_send_action))
			f = meshpl_send_action[act];
		break;
	case IEEE80211_ACTION_CAT_MESH:
		if (act < N(meshaction_send_action))
			f = meshaction_send_action[act];
		break;
	case IEEE80211_ACTION_CAT_VENDOR:
		if (act < N(vendor_send_action))
			f = vendor_send_action[act];
		break;
	}
	return f(ni, cat, act, sa);
#undef N
}

static int
recv_inval(struct ieee80211_node *ni, const struct ieee80211_frame *wh,
	const uint8_t *frm, const uint8_t *efrm)
{
	return EINVAL;
}

static ieee80211_recv_action_func *ba_recv_action[8] = {
	recv_inval, recv_inval, recv_inval, recv_inval,
	recv_inval, recv_inval, recv_inval, recv_inval,
};
static ieee80211_recv_action_func *ht_recv_action[8] = {
	recv_inval, recv_inval, recv_inval, recv_inval,
	recv_inval, recv_inval, recv_inval, recv_inval,
};
static ieee80211_recv_action_func *meshpl_recv_action[8] = {
	recv_inval, recv_inval, recv_inval, recv_inval,
	recv_inval, recv_inval, recv_inval, recv_inval,
};
static ieee80211_recv_action_func *meshaction_recv_action[12] = {
	recv_inval, recv_inval, recv_inval, recv_inval,
	recv_inval, recv_inval, recv_inval, recv_inval,
	recv_inval, recv_inval, recv_inval, recv_inval,
};
static ieee80211_recv_action_func *vendor_recv_action[8] = {
	recv_inval, recv_inval, recv_inval, recv_inval,
	recv_inval, recv_inval, recv_inval, recv_inval,
};

int
ieee80211_recv_action_register(int cat, int act, ieee80211_recv_action_func *f)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	switch (cat) {
	case IEEE80211_ACTION_CAT_BA:
		if (act >= N(ba_recv_action))
			break;
		ba_recv_action[act] = f;
		return 0;
	case IEEE80211_ACTION_CAT_HT:
		if (act >= N(ht_recv_action))
			break;
		ht_recv_action[act] = f;
		return 0;
	case IEEE80211_ACTION_CAT_SELF_PROT:
		if (act >= N(meshpl_recv_action))
			break;
		meshpl_recv_action[act] = f;
		return 0;
	case IEEE80211_ACTION_CAT_MESH:
		if (act >= N(meshaction_recv_action))
			break;
		meshaction_recv_action[act] = f;
		return 0;
	case IEEE80211_ACTION_CAT_VENDOR:
		if (act >= N(vendor_recv_action))
			break;
		vendor_recv_action[act] = f;
		return 0;
	}
	return EINVAL;
#undef N
}

void
ieee80211_recv_action_unregister(int cat, int act)
{
	ieee80211_recv_action_register(cat, act, recv_inval);
}

int
ieee80211_recv_action(struct ieee80211_node *ni,
	const struct ieee80211_frame *wh,
	const uint8_t *frm, const uint8_t *efrm)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	ieee80211_recv_action_func *f = recv_inval;
	struct ieee80211vap *vap = ni->ni_vap;
	const struct ieee80211_action *ia =
	    (const struct ieee80211_action *) frm;

	switch (ia->ia_category) {
	case IEEE80211_ACTION_CAT_BA:
		if (ia->ia_action < N(ba_recv_action))
			f = ba_recv_action[ia->ia_action];
		break;
	case IEEE80211_ACTION_CAT_HT:
		if (ia->ia_action < N(ht_recv_action))
			f = ht_recv_action[ia->ia_action];
		break;
	case IEEE80211_ACTION_CAT_SELF_PROT:
		if (ia->ia_action < N(meshpl_recv_action))
			f = meshpl_recv_action[ia->ia_action];
		break;
	case IEEE80211_ACTION_CAT_MESH:
		if (ni == vap->iv_bss ||
		    ni->ni_mlstate != IEEE80211_NODE_MESH_ESTABLISHED) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_MESH,
			    ni->ni_macaddr, NULL,
			    "peer link not yet established (%d), cat %s act %u",
			    ni->ni_mlstate, "mesh action", ia->ia_action);
			vap->iv_stats.is_mesh_nolink++;
			break;
		}
		if (ia->ia_action < N(meshaction_recv_action))
			f = meshaction_recv_action[ia->ia_action];
		break;
	case IEEE80211_ACTION_CAT_VENDOR:
		if (ia->ia_action < N(vendor_recv_action))
			f = vendor_recv_action[ia->ia_action];
		break;
	}
	return f(ni, wh, frm, efrm);
#undef N
}