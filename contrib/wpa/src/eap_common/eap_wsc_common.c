
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
#include "eap_defs.h"
#include "eap_common.h"
#include "wps/wps.h"
#include "eap_wsc_common.h"

struct wpabuf * eap_wsc_build_frag_ack(u8 id, u8 code)
{
	struct wpabuf *msg;

	msg = eap_msg_alloc(EAP_VENDOR_WFA, EAP_VENDOR_TYPE_WSC, 2, code, id);
	if (msg == NULL) {
		wpa_printf(MSG_ERROR, "EAP-WSC: Failed to allocate memory for "
			   "FRAG_ACK");
		return NULL;
	}

	wpa_printf(MSG_DEBUG, "EAP-WSC: Send WSC/FRAG_ACK");
	wpabuf_put_u8(msg, WSC_FRAG_ACK); /* Op-Code */
	wpabuf_put_u8(msg, 0); /* Flags */

	return msg;
}