
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_media.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_ratectl.h>

static const struct ieee80211_ratectl *ratectls[IEEE80211_RATECTL_MAX];

static const char *ratectl_modnames[IEEE80211_RATECTL_MAX] = {
	[IEEE80211_RATECTL_AMRR]	= "wlan_amrr",
	[IEEE80211_RATECTL_RSSADAPT]	= "wlan_rssadapt",
	[IEEE80211_RATECTL_ONOE]	= "wlan_onoe",
	[IEEE80211_RATECTL_SAMPLE]	= "wlan_sample",
	[IEEE80211_RATECTL_NONE]	= "wlan_none",
};

MALLOC_DEFINE(M_80211_RATECTL, "80211ratectl", "802.11 rate control");

void
ieee80211_ratectl_register(int type, const struct ieee80211_ratectl *ratectl)
{
	if (type >= IEEE80211_RATECTL_MAX)
		return;
	ratectls[type] = ratectl;
}

void
ieee80211_ratectl_unregister(int type)
{
	if (type >= IEEE80211_RATECTL_MAX)
		return;
	ratectls[type] = NULL;
}

void
ieee80211_ratectl_init(struct ieee80211vap *vap)
{
	if (vap->iv_rate == ratectls[IEEE80211_RATECTL_NONE])
		ieee80211_ratectl_set(vap, IEEE80211_RATECTL_AMRR);
	vap->iv_rate->ir_init(vap);
}

void
ieee80211_ratectl_set(struct ieee80211vap *vap, int type)
{
	if (type >= IEEE80211_RATECTL_MAX)
		return;
	if (ratectls[type] == NULL) {
		ieee80211_load_module(ratectl_modnames[type]);
		if (ratectls[type] == NULL) {
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_RATECTL,
			    "%s: unable to load algo %u, module %s\n",
			    __func__, type, ratectl_modnames[type]);
			vap->iv_rate = ratectls[IEEE80211_RATECTL_NONE];
			return;
		}
	}
	vap->iv_rate = ratectls[type];
}