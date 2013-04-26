
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
krb5_rd_rep(krb5_context context,
	    krb5_auth_context auth_context,
	    const krb5_data *inbuf,
	    krb5_ap_rep_enc_part **repl)
{
    krb5_error_code ret;
    AP_REP ap_rep;
    size_t len;
    krb5_data data;
    krb5_crypto crypto;

    krb5_data_zero (&data);

    ret = decode_AP_REP(inbuf->data, inbuf->length, &ap_rep, &len);
    if (ret)
	return ret;
    if (ap_rep.pvno != 5) {
	ret = KRB5KRB_AP_ERR_BADVERSION;
	krb5_clear_error_message (context);
	goto out;
    }
    if (ap_rep.msg_type != krb_ap_rep) {
	ret = KRB5KRB_AP_ERR_MSG_TYPE;
	krb5_clear_error_message (context);
	goto out;
    }

    ret = krb5_crypto_init(context, auth_context->keyblock, 0, &crypto);
    if (ret)
	goto out;
    ret = krb5_decrypt_EncryptedData (context,
				      crypto,
				      KRB5_KU_AP_REQ_ENC_PART,
				      &ap_rep.enc_part,
				      &data);
    krb5_crypto_destroy(context, crypto);
    if (ret)
	goto out;

    *repl = malloc(sizeof(**repl));
    if (*repl == NULL) {
	ret = ENOMEM;
	krb5_set_error_message(context, ret, N_("malloc: out of memory", ""));
	goto out;
    }
    ret = decode_EncAPRepPart(data.data, data.length, *repl, &len);
    if (ret) {
	krb5_set_error_message(context, ret, N_("Failed to decode EncAPRepPart", ""));
	return ret;
    }

    if (auth_context->flags & KRB5_AUTH_CONTEXT_DO_TIME) {
	if ((*repl)->ctime != auth_context->authenticator->ctime ||
	    (*repl)->cusec != auth_context->authenticator->cusec)
	{
	    krb5_free_ap_rep_enc_part(context, *repl);
	    *repl = NULL;
	    ret = KRB5KRB_AP_ERR_MUT_FAIL;
	    krb5_clear_error_message (context);
	    goto out;
	}
    }
    if ((*repl)->seq_number)
	krb5_auth_con_setremoteseqnumber(context, auth_context,
					 *((*repl)->seq_number));
    if ((*repl)->subkey)
	krb5_auth_con_setremotesubkey(context, auth_context, (*repl)->subkey);

 out:
    krb5_data_free (&data);
    free_AP_REP (&ap_rep);
    return ret;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_ap_rep_enc_part (krb5_context context,
			   krb5_ap_rep_enc_part *val)
{
    if (val) {
	free_EncAPRepPart (val);
	free (val);
    }
}