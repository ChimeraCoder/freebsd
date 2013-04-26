
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

/*
 * IEEE 802.11 NULL crypto support.
 */
#include "opt_wlan.h"

#include <sys/param.h>
#include <sys/kernel.h> 
#include <sys/systm.h> 
#include <sys/mbuf.h>   
#include <sys/module.h>

#include <sys/socket.h>

#include <net/if.h>
#include <net/if_media.h>
#include <net/ethernet.h>

#include <net80211/ieee80211_var.h>

static	void *none_attach(struct ieee80211vap *, struct ieee80211_key *);
static	void none_detach(struct ieee80211_key *);
static	int none_setkey(struct ieee80211_key *);
static	int none_encap(struct ieee80211_key *, struct mbuf *, uint8_t);
static	int none_decap(struct ieee80211_key *, struct mbuf *, int);
static	int none_enmic(struct ieee80211_key *, struct mbuf *, int);
static	int none_demic(struct ieee80211_key *, struct mbuf *, int);

const struct ieee80211_cipher ieee80211_cipher_none = {
	.ic_name	= "NONE",
	.ic_cipher	= IEEE80211_CIPHER_NONE,
	.ic_header	= 0,
	.ic_trailer	= 0,
	.ic_miclen	= 0,
	.ic_attach	= none_attach,
	.ic_detach	= none_detach,
	.ic_setkey	= none_setkey,
	.ic_encap	= none_encap,
	.ic_decap	= none_decap,
	.ic_enmic	= none_enmic,
	.ic_demic	= none_demic,
};

static void *
none_attach(struct ieee80211vap *vap, struct ieee80211_key *k)
{
	return vap;		/* for diagnostics+stats */
}

static void
none_detach(struct ieee80211_key *k)
{
	(void) k;
}

static int
none_setkey(struct ieee80211_key *k)
{
	(void) k;
	return 1;
}

static int
none_encap(struct ieee80211_key *k, struct mbuf *m, uint8_t keyid)
{
	struct ieee80211vap *vap = k->wk_private;
#ifdef IEEE80211_DEBUG
	struct ieee80211_frame *wh = mtod(m, struct ieee80211_frame *);
#endif

	/*
	 * The specified key is not setup; this can
	 * happen, at least, when changing keys.
	 */
	IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_CRYPTO, wh->i_addr1,
	    "key id %u is not set (encap)", keyid>>6);
	vap->iv_stats.is_tx_badcipher++;
	return 0;
}

static int
none_decap(struct ieee80211_key *k, struct mbuf *m, int hdrlen)
{
	struct ieee80211vap *vap = k->wk_private;
#ifdef IEEE80211_DEBUG
	struct ieee80211_frame *wh = mtod(m, struct ieee80211_frame *);
	const uint8_t *ivp = (const uint8_t *)&wh[1];
#endif

	/*
	 * The specified key is not setup; this can
	 * happen, at least, when changing keys.
	 */
	/* XXX useful to know dst too */
	IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_CRYPTO, wh->i_addr2,
	    "key id %u is not set (decap)", ivp[IEEE80211_WEP_IVLEN] >> 6);
	vap->iv_stats.is_rx_badkeyid++;
	return 0;
}

static int
none_enmic(struct ieee80211_key *k, struct mbuf *m, int force)
{
	struct ieee80211vap *vap = k->wk_private;

	vap->iv_stats.is_tx_badcipher++;
	return 0;
}

static int
none_demic(struct ieee80211_key *k, struct mbuf *m, int force)
{
	struct ieee80211vap *vap = k->wk_private;

	vap->iv_stats.is_rx_badkeyid++;
	return 0;
}