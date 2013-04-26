
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

/*
 * ARCFOUR
 */

#include "krb5_locl.h"

static struct _krb5_key_type keytype_arcfour = {
    ENCTYPE_ARCFOUR_HMAC_MD5,
    "arcfour",
    128,
    16,
    sizeof(struct _krb5_evp_schedule),
    NULL,
    _krb5_evp_schedule,
    _krb5_arcfour_salt,
    NULL,
    _krb5_evp_cleanup,
    EVP_rc4
};

/*
 * checksum according to section 5. of draft-brezak-win2k-krb-rc4-hmac-03.txt
 */

krb5_error_code
_krb5_HMAC_MD5_checksum(krb5_context context,
			struct _krb5_key_data *key,
			const void *data,
			size_t len,
			unsigned usage,
			Checksum *result)
{
    EVP_MD_CTX *m;
    struct _krb5_checksum_type *c = _krb5_find_checksum (CKSUMTYPE_RSA_MD5);
    const char signature[] = "signaturekey";
    Checksum ksign_c;
    struct _krb5_key_data ksign;
    krb5_keyblock kb;
    unsigned char t[4];
    unsigned char tmp[16];
    unsigned char ksign_c_data[16];
    krb5_error_code ret;

    m = EVP_MD_CTX_create();
    if (m == NULL) {
	krb5_set_error_message(context, ENOMEM, N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    ksign_c.checksum.length = sizeof(ksign_c_data);
    ksign_c.checksum.data   = ksign_c_data;
    ret = _krb5_internal_hmac(context, c, signature, sizeof(signature),
			      0, key, &ksign_c);
    if (ret) {
	EVP_MD_CTX_destroy(m);
	return ret;
    }
    ksign.key = &kb;
    kb.keyvalue = ksign_c.checksum;
    EVP_DigestInit_ex(m, EVP_md5(), NULL);
    t[0] = (usage >>  0) & 0xFF;
    t[1] = (usage >>  8) & 0xFF;
    t[2] = (usage >> 16) & 0xFF;
    t[3] = (usage >> 24) & 0xFF;
    EVP_DigestUpdate(m, t, 4);
    EVP_DigestUpdate(m, data, len);
    EVP_DigestFinal_ex (m, tmp, NULL);
    EVP_MD_CTX_destroy(m);

    ret = _krb5_internal_hmac(context, c, tmp, sizeof(tmp), 0, &ksign, result);
    if (ret)
	return ret;
    return 0;
}

struct _krb5_checksum_type _krb5_checksum_hmac_md5 = {
    CKSUMTYPE_HMAC_MD5,
    "hmac-md5",
    64,
    16,
    F_KEYED | F_CPROOF,
    _krb5_HMAC_MD5_checksum,
    NULL
};

/*
 * section 6 of draft-brezak-win2k-krb-rc4-hmac-03
 *
 * warning: not for small children
 */

static krb5_error_code
ARCFOUR_subencrypt(krb5_context context,
		   struct _krb5_key_data *key,
		   void *data,
		   size_t len,
		   unsigned usage,
		   void *ivec)
{
    EVP_CIPHER_CTX ctx;
    struct _krb5_checksum_type *c = _krb5_find_checksum (CKSUMTYPE_RSA_MD5);
    Checksum k1_c, k2_c, k3_c, cksum;
    struct _krb5_key_data ke;
    krb5_keyblock kb;
    unsigned char t[4];
    unsigned char *cdata = data;
    unsigned char k1_c_data[16], k2_c_data[16], k3_c_data[16];
    krb5_error_code ret;

    t[0] = (usage >>  0) & 0xFF;
    t[1] = (usage >>  8) & 0xFF;
    t[2] = (usage >> 16) & 0xFF;
    t[3] = (usage >> 24) & 0xFF;

    k1_c.checksum.length = sizeof(k1_c_data);
    k1_c.checksum.data   = k1_c_data;

    ret = _krb5_internal_hmac(NULL, c, t, sizeof(t), 0, key, &k1_c);
    if (ret)
	krb5_abortx(context, "hmac failed");

    memcpy (k2_c_data, k1_c_data, sizeof(k1_c_data));

    k2_c.checksum.length = sizeof(k2_c_data);
    k2_c.checksum.data   = k2_c_data;

    ke.key = &kb;
    kb.keyvalue = k2_c.checksum;

    cksum.checksum.length = 16;
    cksum.checksum.data   = data;

    ret = _krb5_internal_hmac(NULL, c, cdata + 16, len - 16, 0, &ke, &cksum);
    if (ret)
	krb5_abortx(context, "hmac failed");

    ke.key = &kb;
    kb.keyvalue = k1_c.checksum;

    k3_c.checksum.length = sizeof(k3_c_data);
    k3_c.checksum.data   = k3_c_data;

    ret = _krb5_internal_hmac(NULL, c, data, 16, 0, &ke, &k3_c);
    if (ret)
	krb5_abortx(context, "hmac failed");

    EVP_CIPHER_CTX_init(&ctx);

    EVP_CipherInit_ex(&ctx, EVP_rc4(), NULL, k3_c.checksum.data, NULL, 1);
    EVP_Cipher(&ctx, cdata + 16, cdata + 16, len - 16);
    EVP_CIPHER_CTX_cleanup(&ctx);

    memset (k1_c_data, 0, sizeof(k1_c_data));
    memset (k2_c_data, 0, sizeof(k2_c_data));
    memset (k3_c_data, 0, sizeof(k3_c_data));
    return 0;
}

static krb5_error_code
ARCFOUR_subdecrypt(krb5_context context,
		   struct _krb5_key_data *key,
		   void *data,
		   size_t len,
		   unsigned usage,
		   void *ivec)
{
    EVP_CIPHER_CTX ctx;
    struct _krb5_checksum_type *c = _krb5_find_checksum (CKSUMTYPE_RSA_MD5);
    Checksum k1_c, k2_c, k3_c, cksum;
    struct _krb5_key_data ke;
    krb5_keyblock kb;
    unsigned char t[4];
    unsigned char *cdata = data;
    unsigned char k1_c_data[16], k2_c_data[16], k3_c_data[16];
    unsigned char cksum_data[16];
    krb5_error_code ret;

    t[0] = (usage >>  0) & 0xFF;
    t[1] = (usage >>  8) & 0xFF;
    t[2] = (usage >> 16) & 0xFF;
    t[3] = (usage >> 24) & 0xFF;

    k1_c.checksum.length = sizeof(k1_c_data);
    k1_c.checksum.data   = k1_c_data;

    ret = _krb5_internal_hmac(NULL, c, t, sizeof(t), 0, key, &k1_c);
    if (ret)
	krb5_abortx(context, "hmac failed");

    memcpy (k2_c_data, k1_c_data, sizeof(k1_c_data));

    k2_c.checksum.length = sizeof(k2_c_data);
    k2_c.checksum.data   = k2_c_data;

    ke.key = &kb;
    kb.keyvalue = k1_c.checksum;

    k3_c.checksum.length = sizeof(k3_c_data);
    k3_c.checksum.data   = k3_c_data;

    ret = _krb5_internal_hmac(NULL, c, cdata, 16, 0, &ke, &k3_c);
    if (ret)
	krb5_abortx(context, "hmac failed");

    EVP_CIPHER_CTX_init(&ctx);
    EVP_CipherInit_ex(&ctx, EVP_rc4(), NULL, k3_c.checksum.data, NULL, 0);
    EVP_Cipher(&ctx, cdata + 16, cdata + 16, len - 16);
    EVP_CIPHER_CTX_cleanup(&ctx);

    ke.key = &kb;
    kb.keyvalue = k2_c.checksum;

    cksum.checksum.length = 16;
    cksum.checksum.data   = cksum_data;

    ret = _krb5_internal_hmac(NULL, c, cdata + 16, len - 16, 0, &ke, &cksum);
    if (ret)
	krb5_abortx(context, "hmac failed");

    memset (k1_c_data, 0, sizeof(k1_c_data));
    memset (k2_c_data, 0, sizeof(k2_c_data));
    memset (k3_c_data, 0, sizeof(k3_c_data));

    if (ct_memcmp (cksum.checksum.data, data, 16) != 0) {
	krb5_clear_error_message (context);
	return KRB5KRB_AP_ERR_BAD_INTEGRITY;
    } else {
	return 0;
    }
}

/*
 * convert the usage numbers used in
 * draft-ietf-cat-kerb-key-derivation-00.txt to the ones in
 * draft-brezak-win2k-krb-rc4-hmac-04.txt
 */

krb5_error_code
_krb5_usage2arcfour(krb5_context context, unsigned *usage)
{
    switch (*usage) {
    case KRB5_KU_AS_REP_ENC_PART : /* 3 */
	*usage = 8;
	return 0;
    case KRB5_KU_USAGE_SEAL :  /* 22 */
	*usage = 13;
	return 0;
    case KRB5_KU_USAGE_SIGN : /* 23 */
        *usage = 15;
        return 0;
    case KRB5_KU_USAGE_SEQ: /* 24 */
	*usage = 0;
	return 0;
    default :
	return 0;
    }
}

static krb5_error_code
ARCFOUR_encrypt(krb5_context context,
		struct _krb5_key_data *key,
		void *data,
		size_t len,
		krb5_boolean encryptp,
		int usage,
		void *ivec)
{
    krb5_error_code ret;
    unsigned keyusage = usage;

    if((ret = _krb5_usage2arcfour (context, &keyusage)) != 0)
	return ret;

    if (encryptp)
	return ARCFOUR_subencrypt (context, key, data, len, keyusage, ivec);
    else
	return ARCFOUR_subdecrypt (context, key, data, len, keyusage, ivec);
}

struct _krb5_encryption_type _krb5_enctype_arcfour_hmac_md5 = {
    ETYPE_ARCFOUR_HMAC_MD5,
    "arcfour-hmac-md5",
    1,
    1,
    8,
    &keytype_arcfour,
    &_krb5_checksum_hmac_md5,
    &_krb5_checksum_hmac_md5,
    F_SPECIAL,
    ARCFOUR_encrypt,
    0,
    NULL
};