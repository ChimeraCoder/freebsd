
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

#include "gsskrb5_locl.h"

#ifdef HEIM_WEAK_CRYPTO

static OM_uint32
mic_des
           (OM_uint32 * minor_status,
            const gsskrb5_ctx ctx,
	    krb5_context context,
            gss_qop_t qop_req,
            const gss_buffer_t message_buffer,
            gss_buffer_t message_token,
	    krb5_keyblock *key
           )
{
  u_char *p;
  EVP_MD_CTX *md5;
  u_char hash[16];
  DES_key_schedule schedule;
  EVP_CIPHER_CTX des_ctx;
  DES_cblock deskey;
  DES_cblock zero;
  int32_t seq_number;
  size_t len, total_len;

  _gsskrb5_encap_length (22, &len, &total_len, GSS_KRB5_MECHANISM);

  message_token->length = total_len;
  message_token->value  = malloc (total_len);
  if (message_token->value == NULL) {
    message_token->length = 0;
    *minor_status = ENOMEM;
    return GSS_S_FAILURE;
  }

  p = _gsskrb5_make_header(message_token->value,
			      len,
			      "\x01\x01", /* TOK_ID */
			      GSS_KRB5_MECHANISM);

  memcpy (p, "\x00\x00", 2);	/* SGN_ALG = DES MAC MD5 */
  p += 2;

  memcpy (p, "\xff\xff\xff\xff", 4); /* Filler */
  p += 4;

  /* Fill in later (SND-SEQ) */
  memset (p, 0, 16);
  p += 16;

  /* checksum */
  md5 = EVP_MD_CTX_create();
  EVP_DigestInit_ex(md5, EVP_md5(), NULL);
  EVP_DigestUpdate(md5, p - 24, 8);
  EVP_DigestUpdate(md5, message_buffer->value, message_buffer->length);
  EVP_DigestFinal_ex(md5, hash, NULL);
  EVP_MD_CTX_destroy(md5);

  memset (&zero, 0, sizeof(zero));
  memcpy (&deskey, key->keyvalue.data, sizeof(deskey));
  DES_set_key_unchecked (&deskey, &schedule);
  DES_cbc_cksum ((void *)hash, (void *)hash, sizeof(hash),
		 &schedule, &zero);
  memcpy (p - 8, hash, 8);	/* SGN_CKSUM */

  HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);
  /* sequence number */
  krb5_auth_con_getlocalseqnumber (context,
				   ctx->auth_context,
				   &seq_number);

  p -= 16;			/* SND_SEQ */
  p[0] = (seq_number >> 0)  & 0xFF;
  p[1] = (seq_number >> 8)  & 0xFF;
  p[2] = (seq_number >> 16) & 0xFF;
  p[3] = (seq_number >> 24) & 0xFF;
  memset (p + 4,
	  (ctx->more_flags & LOCAL) ? 0 : 0xFF,
	  4);

  EVP_CIPHER_CTX_init(&des_ctx);
  EVP_CipherInit_ex(&des_ctx, EVP_des_cbc(), NULL, key->keyvalue.data, p + 8, 1);
  EVP_Cipher(&des_ctx, p, p, 8);
  EVP_CIPHER_CTX_cleanup(&des_ctx);

  krb5_auth_con_setlocalseqnumber (context,
			       ctx->auth_context,
			       ++seq_number);
  HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);

  memset (deskey, 0, sizeof(deskey));
  memset (&schedule, 0, sizeof(schedule));

  *minor_status = 0;
  return GSS_S_COMPLETE;
}
#endif

static OM_uint32
mic_des3
           (OM_uint32 * minor_status,
            const gsskrb5_ctx ctx,
	    krb5_context context,
            gss_qop_t qop_req,
            const gss_buffer_t message_buffer,
            gss_buffer_t message_token,
	    krb5_keyblock *key
           )
{
  u_char *p;
  Checksum cksum;
  u_char seq[8];

  int32_t seq_number;
  size_t len, total_len;

  krb5_crypto crypto;
  krb5_error_code kret;
  krb5_data encdata;
  char *tmp;
  char ivec[8];

  _gsskrb5_encap_length (36, &len, &total_len, GSS_KRB5_MECHANISM);

  message_token->length = total_len;
  message_token->value  = malloc (total_len);
  if (message_token->value == NULL) {
      message_token->length = 0;
      *minor_status = ENOMEM;
      return GSS_S_FAILURE;
  }

  p = _gsskrb5_make_header(message_token->value,
			      len,
			      "\x01\x01", /* TOK-ID */
			      GSS_KRB5_MECHANISM);

  memcpy (p, "\x04\x00", 2);	/* SGN_ALG = HMAC SHA1 DES3-KD */
  p += 2;

  memcpy (p, "\xff\xff\xff\xff", 4); /* filler */
  p += 4;

  /* this should be done in parts */

  tmp = malloc (message_buffer->length + 8);
  if (tmp == NULL) {
      free (message_token->value);
      message_token->value = NULL;
      message_token->length = 0;
      *minor_status = ENOMEM;
      return GSS_S_FAILURE;
  }
  memcpy (tmp, p - 8, 8);
  memcpy (tmp + 8, message_buffer->value, message_buffer->length);

  kret = krb5_crypto_init(context, key, 0, &crypto);
  if (kret) {
      free (message_token->value);
      message_token->value = NULL;
      message_token->length = 0;
      free (tmp);
      *minor_status = kret;
      return GSS_S_FAILURE;
  }

  kret = krb5_create_checksum (context,
			       crypto,
			       KRB5_KU_USAGE_SIGN,
			       0,
			       tmp,
			       message_buffer->length + 8,
			       &cksum);
  free (tmp);
  krb5_crypto_destroy (context, crypto);
  if (kret) {
      free (message_token->value);
      message_token->value = NULL;
      message_token->length = 0;
      *minor_status = kret;
      return GSS_S_FAILURE;
  }

  memcpy (p + 8, cksum.checksum.data, cksum.checksum.length);

  HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);
  /* sequence number */
  krb5_auth_con_getlocalseqnumber (context,
			       ctx->auth_context,
			       &seq_number);

  seq[0] = (seq_number >> 0)  & 0xFF;
  seq[1] = (seq_number >> 8)  & 0xFF;
  seq[2] = (seq_number >> 16) & 0xFF;
  seq[3] = (seq_number >> 24) & 0xFF;
  memset (seq + 4,
	  (ctx->more_flags & LOCAL) ? 0 : 0xFF,
	  4);

  kret = krb5_crypto_init(context, key,
			  ETYPE_DES3_CBC_NONE, &crypto);
  if (kret) {
      free (message_token->value);
      message_token->value = NULL;
      message_token->length = 0;
      *minor_status = kret;
      return GSS_S_FAILURE;
  }

  if (ctx->more_flags & COMPAT_OLD_DES3)
      memset(ivec, 0, 8);
  else
      memcpy(ivec, p + 8, 8);

  kret = krb5_encrypt_ivec (context,
			    crypto,
			    KRB5_KU_USAGE_SEQ,
			    seq, 8, &encdata, ivec);
  krb5_crypto_destroy (context, crypto);
  if (kret) {
      free (message_token->value);
      message_token->value = NULL;
      message_token->length = 0;
      *minor_status = kret;
      return GSS_S_FAILURE;
  }

  assert (encdata.length == 8);

  memcpy (p, encdata.data, encdata.length);
  krb5_data_free (&encdata);

  krb5_auth_con_setlocalseqnumber (context,
			       ctx->auth_context,
			       ++seq_number);
  HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);

  free_Checksum (&cksum);
  *minor_status = 0;
  return GSS_S_COMPLETE;
}

OM_uint32 GSSAPI_CALLCONV _gsskrb5_get_mic
           (OM_uint32 * minor_status,
            const gss_ctx_id_t context_handle,
            gss_qop_t qop_req,
            const gss_buffer_t message_buffer,
            gss_buffer_t message_token
           )
{
  krb5_context context;
  const gsskrb5_ctx ctx = (const gsskrb5_ctx) context_handle;
  krb5_keyblock *key;
  OM_uint32 ret;
  krb5_keytype keytype;

  GSSAPI_KRB5_INIT (&context);

  if (ctx->more_flags & IS_CFX)
      return _gssapi_mic_cfx (minor_status, ctx, context, qop_req,
			      message_buffer, message_token);

  HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);
  ret = _gsskrb5i_get_token_key(ctx, context, &key);
  HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);
  if (ret) {
      *minor_status = ret;
      return GSS_S_FAILURE;
  }
  krb5_enctype_to_keytype (context, key->keytype, &keytype);

  switch (keytype) {
  case KEYTYPE_DES :
#ifdef HEIM_WEAK_CRYPTO
      ret = mic_des (minor_status, ctx, context, qop_req,
		     message_buffer, message_token, key);
#else
      ret = GSS_S_FAILURE;
#endif
      break;
  case KEYTYPE_DES3 :
      ret = mic_des3 (minor_status, ctx, context, qop_req,
		      message_buffer, message_token, key);
      break;
  case KEYTYPE_ARCFOUR:
  case KEYTYPE_ARCFOUR_56:
      ret = _gssapi_get_mic_arcfour (minor_status, ctx, context, qop_req,
				     message_buffer, message_token, key);
      break;
  default :
      abort();
      break;
  }
  krb5_free_keyblock (context, key);
  return ret;
}