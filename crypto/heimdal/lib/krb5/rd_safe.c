
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

static krb5_error_code
verify_checksum(krb5_context context,
		krb5_auth_context auth_context,
		KRB_SAFE *safe)
{
    krb5_error_code ret;
    u_char *buf;
    size_t buf_size;
    size_t len = 0;
    Checksum c;
    krb5_crypto crypto;
    krb5_keyblock *key;

    c = safe->cksum;
    safe->cksum.cksumtype       = 0;
    safe->cksum.checksum.data   = NULL;
    safe->cksum.checksum.length = 0;

    ASN1_MALLOC_ENCODE(KRB_SAFE, buf, buf_size, safe, &len, ret);
    if(ret)
	return ret;
    if(buf_size != len)
	krb5_abortx(context, "internal error in ASN.1 encoder");

    if (auth_context->remote_subkey)
	key = auth_context->remote_subkey;
    else if (auth_context->local_subkey)
	key = auth_context->local_subkey;
    else
	key = auth_context->keyblock;

    ret = krb5_crypto_init(context, key, 0, &crypto);
    if (ret)
	goto out;
    ret = krb5_verify_checksum (context,
				crypto,
				KRB5_KU_KRB_SAFE_CKSUM,
				buf + buf_size - len,
				len,
				&c);
    krb5_crypto_destroy(context, crypto);
out:
    safe->cksum = c;
    free (buf);
    return ret;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_rd_safe(krb5_context context,
	     krb5_auth_context auth_context,
	     const krb5_data *inbuf,
	     krb5_data *outbuf,
	     krb5_replay_data *outdata)
{
    krb5_error_code ret;
    KRB_SAFE safe;
    size_t len;

    krb5_data_zero(outbuf);

    if ((auth_context->flags &
	 (KRB5_AUTH_CONTEXT_RET_TIME | KRB5_AUTH_CONTEXT_RET_SEQUENCE)))
    {
	if (outdata == NULL) {
	    krb5_set_error_message(context, KRB5_RC_REQUIRED,
				   N_("rd_safe: need outdata "
				      "to return data", ""));
	    return KRB5_RC_REQUIRED; /* XXX better error, MIT returns this */
	}
	/* if these fields are not present in the safe-part, silently
           return zero */
	memset(outdata, 0, sizeof(*outdata));
    }

    ret = decode_KRB_SAFE (inbuf->data, inbuf->length, &safe, &len);
    if (ret)
	return ret;
    if (safe.pvno != 5) {
	ret = KRB5KRB_AP_ERR_BADVERSION;
	krb5_clear_error_message (context);
	goto failure;
    }
    if (safe.msg_type != krb_safe) {
	ret = KRB5KRB_AP_ERR_MSG_TYPE;
	krb5_clear_error_message (context);
	goto failure;
    }
    if (!krb5_checksum_is_keyed(context, safe.cksum.cksumtype)
	|| !krb5_checksum_is_collision_proof(context, safe.cksum.cksumtype)) {
	ret = KRB5KRB_AP_ERR_INAPP_CKSUM;
	krb5_clear_error_message (context);
	goto failure;
    }

    /* check sender address */

    if (safe.safe_body.s_address
	&& auth_context->remote_address
	&& !krb5_address_compare (context,
				  auth_context->remote_address,
				  safe.safe_body.s_address)) {
	ret = KRB5KRB_AP_ERR_BADADDR;
	krb5_clear_error_message (context);
	goto failure;
    }

    /* check receiver address */

    if (safe.safe_body.r_address
	&& auth_context->local_address
	&& !krb5_address_compare (context,
				  auth_context->local_address,
				  safe.safe_body.r_address)) {
	ret = KRB5KRB_AP_ERR_BADADDR;
	krb5_clear_error_message (context);
	goto failure;
    }

    /* check timestamp */
    if (auth_context->flags & KRB5_AUTH_CONTEXT_DO_TIME) {
	krb5_timestamp sec;

	krb5_timeofday (context, &sec);

	if (safe.safe_body.timestamp == NULL ||
	    safe.safe_body.usec      == NULL ||
	    abs(*safe.safe_body.timestamp - sec) > context->max_skew) {
	    ret = KRB5KRB_AP_ERR_SKEW;
	    krb5_clear_error_message (context);
	    goto failure;
	}
    }
    /* XXX - check replay cache */

    /* check sequence number. since MIT krb5 cannot generate a sequence
       number of zero but instead generates no sequence number, we accept that
    */

    if (auth_context->flags & KRB5_AUTH_CONTEXT_DO_SEQUENCE) {
	if ((safe.safe_body.seq_number == NULL
	     && auth_context->remote_seqnumber != 0)
	    || (safe.safe_body.seq_number != NULL
		&& *safe.safe_body.seq_number !=
		auth_context->remote_seqnumber)) {
	    ret = KRB5KRB_AP_ERR_BADORDER;
	    krb5_clear_error_message (context);
	    goto failure;
	}
	auth_context->remote_seqnumber++;
    }

    ret = verify_checksum (context, auth_context, &safe);
    if (ret)
	goto failure;

    outbuf->length = safe.safe_body.user_data.length;
    outbuf->data   = malloc(outbuf->length);
    if (outbuf->data == NULL && outbuf->length != 0) {
	ret = ENOMEM;
	krb5_set_error_message(context, ret, N_("malloc: out of memory", ""));
	krb5_data_zero(outbuf);
	goto failure;
    }
    memcpy (outbuf->data, safe.safe_body.user_data.data, outbuf->length);

    if ((auth_context->flags &
	 (KRB5_AUTH_CONTEXT_RET_TIME | KRB5_AUTH_CONTEXT_RET_SEQUENCE))) {

	if(safe.safe_body.timestamp)
	    outdata->timestamp = *safe.safe_body.timestamp;
	if(safe.safe_body.usec)
	    outdata->usec = *safe.safe_body.usec;
	if(safe.safe_body.seq_number)
	    outdata->seq = *safe.safe_body.seq_number;
    }

  failure:
    free_KRB_SAFE (&safe);
    return ret;
}