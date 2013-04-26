
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

#include "includes.h"

#include "common.h"
#include "eap_i.h"
#include "eap_common/chap.h"


static void * eap_md5_init(struct eap_sm *sm)
{
	/* No need for private data. However, must return non-NULL to indicate
	 * success. */
	return (void *) 1;
}


static void eap_md5_deinit(struct eap_sm *sm, void *priv)
{
}


static struct wpabuf * eap_md5_process(struct eap_sm *sm, void *priv,
				       struct eap_method_ret *ret,
				       const struct wpabuf *reqData)
{
	struct wpabuf *resp;
	const u8 *pos, *challenge, *password;
	u8 *rpos, id;
	size_t len, challenge_len, password_len;

	password = eap_get_config_password(sm, &password_len);
	if (password == NULL) {
		wpa_printf(MSG_INFO, "EAP-MD5: Password not configured");
		eap_sm_request_password(sm);
		ret->ignore = TRUE;
		return NULL;
	}

	pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_MD5, reqData, &len);
	if (pos == NULL || len == 0) {
		wpa_printf(MSG_INFO, "EAP-MD5: Invalid frame (pos=%p len=%lu)",
			   pos, (unsigned long) len);
		ret->ignore = TRUE;
		return NULL;
	}

	/*
	 * CHAP Challenge:
	 * Value-Size (1 octet) | Value(Challenge) | Name(optional)
	 */
	challenge_len = *pos++;
	if (challenge_len == 0 || challenge_len > len - 1) {
		wpa_printf(MSG_INFO, "EAP-MD5: Invalid challenge "
			   "(challenge_len=%lu len=%lu)",
			   (unsigned long) challenge_len, (unsigned long) len);
		ret->ignore = TRUE;
		return NULL;
	}
	ret->ignore = FALSE;
	challenge = pos;
	wpa_hexdump(MSG_MSGDUMP, "EAP-MD5: Challenge",
		    challenge, challenge_len);

	wpa_printf(MSG_DEBUG, "EAP-MD5: Generating Challenge Response");
	ret->methodState = METHOD_DONE;
	ret->decision = DECISION_COND_SUCC;
	ret->allowNotifications = TRUE;

	resp = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_MD5, 1 + CHAP_MD5_LEN,
			     EAP_CODE_RESPONSE, eap_get_id(reqData));
	if (resp == NULL)
		return NULL;

	/*
	 * CHAP Response:
	 * Value-Size (1 octet) | Value(Response) | Name(optional)
	 */
	wpabuf_put_u8(resp, CHAP_MD5_LEN);

	id = eap_get_id(resp);
	rpos = wpabuf_put(resp, CHAP_MD5_LEN);
	chap_md5(id, password, password_len, challenge, challenge_len, rpos);
	wpa_hexdump(MSG_MSGDUMP, "EAP-MD5: Response", rpos, CHAP_MD5_LEN);

	return resp;
}


int eap_peer_md5_register(void)
{
	struct eap_method *eap;
	int ret;

	eap = eap_peer_method_alloc(EAP_PEER_METHOD_INTERFACE_VERSION,
				    EAP_VENDOR_IETF, EAP_TYPE_MD5, "MD5");
	if (eap == NULL)
		return -1;

	eap->init = eap_md5_init;
	eap->deinit = eap_md5_deinit;
	eap->process = eap_md5_process;

	ret = eap_peer_method_register(eap);
	if (ret)
		eap_peer_method_free(eap);
	return ret;
}