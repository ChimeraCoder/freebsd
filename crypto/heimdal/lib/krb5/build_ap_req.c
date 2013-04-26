
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

#include "krb5_locl.h"

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_build_ap_req (krb5_context context,
		   krb5_enctype enctype,
		   krb5_creds *cred,
		   krb5_flags ap_options,
		   krb5_data authenticator,
		   krb5_data *retdata)
{
  krb5_error_code ret = 0;
  AP_REQ ap;
  Ticket t;
  size_t len;

  ap.pvno = 5;
  ap.msg_type = krb_ap_req;
  memset(&ap.ap_options, 0, sizeof(ap.ap_options));
  ap.ap_options.use_session_key = (ap_options & AP_OPTS_USE_SESSION_KEY) > 0;
  ap.ap_options.mutual_required = (ap_options & AP_OPTS_MUTUAL_REQUIRED) > 0;

  ap.ticket.tkt_vno = 5;
  copy_Realm(&cred->server->realm, &ap.ticket.realm);
  copy_PrincipalName(&cred->server->name, &ap.ticket.sname);

  decode_Ticket(cred->ticket.data, cred->ticket.length, &t, &len);
  copy_EncryptedData(&t.enc_part, &ap.ticket.enc_part);
  free_Ticket(&t);

  ap.authenticator.etype = enctype;
  ap.authenticator.kvno  = NULL;
  ap.authenticator.cipher = authenticator;

  ASN1_MALLOC_ENCODE(AP_REQ, retdata->data, retdata->length,
		     &ap, &len, ret);
  if(ret == 0 && retdata->length != len)
      krb5_abortx(context, "internal error in ASN.1 encoder");
  free_AP_REQ(&ap);
  return ret;

}