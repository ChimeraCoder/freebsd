
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

/*
 * AES
 */

static struct _krb5_key_type keytype_aes128 = {
    ENCTYPE_AES128_CTS_HMAC_SHA1_96,
    "aes-128",
    128,
    16,
    sizeof(struct _krb5_evp_schedule),
    NULL,
    _krb5_evp_schedule,
    _krb5_AES_salt,
    NULL,
    _krb5_evp_cleanup,
    EVP_aes_128_cbc
};

static struct _krb5_key_type keytype_aes256 = {
    ENCTYPE_AES256_CTS_HMAC_SHA1_96,
    "aes-256",
    256,
    32,
    sizeof(struct _krb5_evp_schedule),
    NULL,
    _krb5_evp_schedule,
    _krb5_AES_salt,
    NULL,
    _krb5_evp_cleanup,
    EVP_aes_256_cbc
};

struct _krb5_checksum_type _krb5_checksum_hmac_sha1_aes128 = {
    CKSUMTYPE_HMAC_SHA1_96_AES_128,
    "hmac-sha1-96-aes128",
    64,
    12,
    F_KEYED | F_CPROOF | F_DERIVED,
    _krb5_SP_HMAC_SHA1_checksum,
    NULL
};

struct _krb5_checksum_type _krb5_checksum_hmac_sha1_aes256 = {
    CKSUMTYPE_HMAC_SHA1_96_AES_256,
    "hmac-sha1-96-aes256",
    64,
    12,
    F_KEYED | F_CPROOF | F_DERIVED,
    _krb5_SP_HMAC_SHA1_checksum,
    NULL
};

static krb5_error_code
AES_PRF(krb5_context context,
	krb5_crypto crypto,
	const krb5_data *in,
	krb5_data *out)
{
    struct _krb5_checksum_type *ct = crypto->et->checksum;
    krb5_error_code ret;
    Checksum result;
    krb5_keyblock *derived;

    result.cksumtype = ct->type;
    ret = krb5_data_alloc(&result.checksum, ct->checksumsize);
    if (ret) {
	krb5_set_error_message(context, ret, N_("malloc: out memory", ""));
	return ret;
    }

    ret = (*ct->checksum)(context, NULL, in->data, in->length, 0, &result);
    if (ret) {
	krb5_data_free(&result.checksum);
	return ret;
    }

    if (result.checksum.length < crypto->et->blocksize)
	krb5_abortx(context, "internal prf error");

    derived = NULL;
    ret = krb5_derive_key(context, crypto->key.key,
			  crypto->et->type, "prf", 3, &derived);
    if (ret)
	krb5_abortx(context, "krb5_derive_key");

    ret = krb5_data_alloc(out, crypto->et->blocksize);
    if (ret)
	krb5_abortx(context, "malloc failed");

    {
	const EVP_CIPHER *c = (*crypto->et->keytype->evp)();
	EVP_CIPHER_CTX ctx;

	EVP_CIPHER_CTX_init(&ctx); /* ivec all zero */
	EVP_CipherInit_ex(&ctx, c, NULL, derived->keyvalue.data, NULL, 1);
	EVP_Cipher(&ctx, out->data, result.checksum.data,
		   crypto->et->blocksize);
	EVP_CIPHER_CTX_cleanup(&ctx);
    }

    krb5_data_free(&result.checksum);
    krb5_free_keyblock(context, derived);

    return ret;
}

struct _krb5_encryption_type _krb5_enctype_aes128_cts_hmac_sha1 = {
    ETYPE_AES128_CTS_HMAC_SHA1_96,
    "aes128-cts-hmac-sha1-96",
    16,
    1,
    16,
    &keytype_aes128,
    &_krb5_checksum_sha1,
    &_krb5_checksum_hmac_sha1_aes128,
    F_DERIVED,
    _krb5_evp_encrypt_cts,
    16,
    AES_PRF
};

struct _krb5_encryption_type _krb5_enctype_aes256_cts_hmac_sha1 = {
    ETYPE_AES256_CTS_HMAC_SHA1_96,
    "aes256-cts-hmac-sha1-96",
    16,
    1,
    16,
    &keytype_aes256,
    &_krb5_checksum_sha1,
    &_krb5_checksum_hmac_sha1_aes256,
    F_DERIVED,
    _krb5_evp_encrypt_cts,
    16,
    AES_PRF
};