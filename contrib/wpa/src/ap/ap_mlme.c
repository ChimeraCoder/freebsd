
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

#include "utils/includes.h"

#include "utils/common.h"
#include "common/ieee802_11_defs.h"
#include "ieee802_11.h"
#include "wpa_auth.h"
#include "sta_info.h"
#include "ap_mlme.h"


#ifndef CONFIG_NO_HOSTAPD_LOGGER
static const char * mlme_auth_alg_str(int alg)
{
	switch (alg) {
	case WLAN_AUTH_OPEN:
		return "OPEN_SYSTEM";
	case WLAN_AUTH_SHARED_KEY:
		return "SHARED_KEY";
	case WLAN_AUTH_FT:
		return "FT";
	}

	return "unknown";
}
#endif /* CONFIG_NO_HOSTAPD_LOGGER */


/**
 * mlme_authenticate_indication - Report the establishment of an authentication
 * relationship with a specific peer MAC entity
 * @hapd: BSS data
 * @sta: peer STA data
 *
 * MLME calls this function as a result of the establishment of an
 * authentication relationship with a specific peer MAC entity that
 * resulted from an authentication procedure that was initiated by
 * that specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 * AuthenticationType = sta->auth_alg (WLAN_AUTH_OPEN / WLAN_AUTH_SHARED_KEY)
 */
void mlme_authenticate_indication(struct hostapd_data *hapd,
				  struct sta_info *sta)
{
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_MLME,
		       HOSTAPD_LEVEL_DEBUG,
		       "MLME-AUTHENTICATE.indication(" MACSTR ", %s)",
		       MAC2STR(sta->addr), mlme_auth_alg_str(sta->auth_alg));
	if (sta->auth_alg != WLAN_AUTH_FT && !(sta->flags & WLAN_STA_MFP))
		mlme_deletekeys_request(hapd, sta);
}


/**
 * mlme_deauthenticate_indication - Report the invalidation of an
 * authentication relationship with a specific peer MAC entity
 * @hapd: BSS data
 * @sta: Peer STA data
 * @reason_code: ReasonCode from Deauthentication frame
 *
 * MLME calls this function as a result of the invalidation of an
 * authentication relationship with a specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 */
void mlme_deauthenticate_indication(struct hostapd_data *hapd,
				    struct sta_info *sta, u16 reason_code)
{
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_MLME,
		       HOSTAPD_LEVEL_DEBUG,
		       "MLME-DEAUTHENTICATE.indication(" MACSTR ", %d)",
		       MAC2STR(sta->addr), reason_code);
	mlme_deletekeys_request(hapd, sta);
}


/**
 * mlme_associate_indication - Report the establishment of an association with
 * a specific peer MAC entity
 * @hapd: BSS data
 * @sta: peer STA data
 *
 * MLME calls this function as a result of the establishment of an
 * association with a specific peer MAC entity that resulted from an
 * association procedure that was initiated by that specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 */
void mlme_associate_indication(struct hostapd_data *hapd, struct sta_info *sta)
{
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_MLME,
		       HOSTAPD_LEVEL_DEBUG,
		       "MLME-ASSOCIATE.indication(" MACSTR ")",
		       MAC2STR(sta->addr));
	if (sta->auth_alg != WLAN_AUTH_FT)
		mlme_deletekeys_request(hapd, sta);
}


/**
 * mlme_reassociate_indication - Report the establishment of an reassociation
 * with a specific peer MAC entity
 * @hapd: BSS data
 * @sta: peer STA data
 *
 * MLME calls this function as a result of the establishment of an
 * reassociation with a specific peer MAC entity that resulted from a
 * reassociation procedure that was initiated by that specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 *
 * sta->previous_ap contains the "Current AP" information from ReassocReq.
 */
void mlme_reassociate_indication(struct hostapd_data *hapd,
				 struct sta_info *sta)
{
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_MLME,
		       HOSTAPD_LEVEL_DEBUG,
		       "MLME-REASSOCIATE.indication(" MACSTR ")",
		       MAC2STR(sta->addr));
	if (sta->auth_alg != WLAN_AUTH_FT)
		mlme_deletekeys_request(hapd, sta);
}


/**
 * mlme_disassociate_indication - Report disassociation with a specific peer
 * MAC entity
 * @hapd: BSS data
 * @sta: Peer STA data
 * @reason_code: ReasonCode from Disassociation frame
 *
 * MLME calls this function as a result of the invalidation of an association
 * relationship with a specific peer MAC entity.
 *
 * PeerSTAAddress = sta->addr
 */
void mlme_disassociate_indication(struct hostapd_data *hapd,
				  struct sta_info *sta, u16 reason_code)
{
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_MLME,
		       HOSTAPD_LEVEL_DEBUG,
		       "MLME-DISASSOCIATE.indication(" MACSTR ", %d)",
		       MAC2STR(sta->addr), reason_code);
	mlme_deletekeys_request(hapd, sta);
}


void mlme_michaelmicfailure_indication(struct hostapd_data *hapd,
				       const u8 *addr)
{
	hostapd_logger(hapd, addr, HOSTAPD_MODULE_MLME,
		       HOSTAPD_LEVEL_DEBUG,
		       "MLME-MichaelMICFailure.indication(" MACSTR ")",
		       MAC2STR(addr));
}


void mlme_deletekeys_request(struct hostapd_data *hapd, struct sta_info *sta)
{
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_MLME,
		       HOSTAPD_LEVEL_DEBUG,
		       "MLME-DELETEKEYS.request(" MACSTR ")",
		       MAC2STR(sta->addr));

	if (sta->wpa_sm)
		wpa_remove_ptk(sta->wpa_sm);
}