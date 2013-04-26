
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

#include "opt_wlan.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/if_media.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/if_ether.h>
#endif

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_ratectl.h>

static void
none_init(struct ieee80211vap *vap)
{
}

static void
none_deinit(struct ieee80211vap *vap)
{
	free(vap->iv_rs, M_80211_RATECTL);
}

static void
none_node_init(struct ieee80211_node *ni)
{
	ni->ni_txrate = ni->ni_rates.rs_rates[0] & IEEE80211_RATE_VAL;
}

static void
none_node_deinit(struct ieee80211_node *ni)
{
}

static int
none_rate(struct ieee80211_node *ni, void *arg __unused, uint32_t iarg __unused)
{
	int rix = 0;

	ni->ni_txrate = ni->ni_rates.rs_rates[rix] & IEEE80211_RATE_VAL;
	return rix;
}

static void
none_tx_complete(const struct ieee80211vap *vap,
    const struct ieee80211_node *ni, int ok,
    void *arg1, void *arg2 __unused)
{
}

static void
none_tx_update(const struct ieee80211vap *vap, const struct ieee80211_node *ni,
    void *arg1, void *arg2, void *arg3)
{
}

static void
none_setinterval(const struct ieee80211vap *vap, int msecs)
{
}

/* number of references from net80211 layer */
static	int nrefs = 0;

static const struct ieee80211_ratectl none = {
	.ir_name	= "none",
	.ir_attach	= NULL,
	.ir_detach	= NULL,
	.ir_init	= none_init,
	.ir_deinit	= none_deinit,
	.ir_node_init	= none_node_init,
	.ir_node_deinit	= none_node_deinit,
	.ir_rate	= none_rate,
	.ir_tx_complete	= none_tx_complete,
	.ir_tx_update	= none_tx_update,
	.ir_setinterval	= none_setinterval,
};
IEEE80211_RATECTL_MODULE(ratectl_none, 1);
IEEE80211_RATECTL_ALG(none, IEEE80211_RATECTL_NONE, none);