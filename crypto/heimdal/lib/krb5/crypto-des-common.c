
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

/* Functions which are used by both single and triple DES enctypes */

#include "krb5_locl.h"

/*
 * A = A xor B. A & B are 8 bytes.
 */

void
_krb5_xor (DES_cblock *key, const unsigned char *b)
{
    unsigned char *a = (unsigned char*)key;
    a[0] ^= b[0];
    a[1] ^= b[1];
    a[2] ^= b[2];
    a[3] ^= b[3];
    a[4] ^= b[4];
    a[5] ^= b[5];
    a[6] ^= b[6];
    a[7] ^= b[7];
}

#if defined(DES3_OLD_ENCTYPE) || defined(HEIM_WEAK_CRYPTO)
krb5_error_code
_krb5_des_checksum(krb5_context context,
		   const EVP_MD *evp_md,
		   struct _krb5_key_data *key,
		   const void *data,
		   size_t len,
		   Checksum *cksum)
{
    struct _krb5_evp_schedule *ctx = key->schedule->data;
    EVP_MD_CTX *m;
    DES_cblock ivec;
    unsigned char *p = cksum->checksum.data;

    krb5_generate_random_block(p, 8);

    m = EVP_MD_CTX_create();
    if (m == NULL) {
	krb5_set_error_message(context, ENOMEM, N_("malloc: out of memory", ""));
	return ENOMEM;
    }

    EVP_DigestInit_ex(m, evp_md, NULL);
    EVP_DigestUpdate(m, p, 8);
    EVP_DigestUpdate(m, data, len);
    EVP_DigestFinal_ex (m, p + 8, NULL);
    EVP_MD_CTX_destroy(m);
    memset (&ivec, 0, sizeof(ivec));
    EVP_CipherInit_ex(&ctx->ectx, NULL, NULL, NULL, (void *)&ivec, -1);
    EVP_Cipher(&ctx->ectx, p, p, 24);

    return 0;
}

krb5_error_code
_krb5_des_verify(krb5_context context,
		 const EVP_MD *evp_md,
		 struct _krb5_key_data *key,
		 const void *data,
		 size_t len,
		 Checksum *C)
{
    struct _krb5_evp_schedule *ctx = key->schedule->data;
    EVP_MD_CTX *m;
    unsigned char tmp[24];
    unsigned char res[16];
    DES_cblock ivec;
    krb5_error_code ret = 0;

    m = EVP_MD_CTX_create();
    if (m == NULL) {
	krb5_set_error_message(context, ENOMEM, N_("malloc: out of memory", ""));
	return ENOMEM;
    }

    memset(&ivec, 0, sizeof(ivec));
    EVP_CipherInit_ex(&ctx->dctx, NULL, NULL, NULL, (void *)&ivec, -1);
    EVP_Cipher(&ctx->dctx, tmp, C->checksum.data, 24);

    EVP_DigestInit_ex(m, evp_md, NULL);
    EVP_DigestUpdate(m, tmp, 8); /* confounder */
    EVP_DigestUpdate(m, data, len);
    EVP_DigestFinal_ex (m, res, NULL);
    EVP_MD_CTX_destroy(m);
    if(ct_memcmp(res, tmp + 8, sizeof(res)) != 0) {
	krb5_clear_error_message (context);
	ret = KRB5KRB_AP_ERR_BAD_INTEGRITY;
    }
    memset(tmp, 0, sizeof(tmp));
    memset(res, 0, sizeof(res));
    return ret;
}

#endif

static krb5_error_code
RSA_MD5_checksum(krb5_context context,
		 struct _krb5_key_data *key,
		 const void *data,
		 size_t len,
		 unsigned usage,
		 Checksum *C)
{
    if (EVP_Digest(data, len, C->checksum.data, NULL, EVP_md5(), NULL) != 1)
	krb5_abortx(context, "md5 checksum failed");
    return 0;
}

struct _krb5_checksum_type _krb5_checksum_rsa_md5 = {
    CKSUMTYPE_RSA_MD5,
    "rsa-md5",
    64,
    16,
    F_CPROOF,
    RSA_MD5_checksum,
    NULL
};