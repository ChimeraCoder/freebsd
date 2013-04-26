
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

#include "cryptlib.h"
#include <openssl/asn1t.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/cms.h>
#include <openssl/rand.h>
#include "cms_lcl.h"

/* CMS EncryptedData Utilities */

DECLARE_ASN1_ITEM(CMS_EncryptedData)

/* Return BIO based on EncryptedContentInfo and key */

BIO *cms_EncryptedContent_init_bio(CMS_EncryptedContentInfo *ec)
	{
	BIO *b;
	EVP_CIPHER_CTX *ctx;
	const EVP_CIPHER *ciph;
	X509_ALGOR *calg = ec->contentEncryptionAlgorithm;
	unsigned char iv[EVP_MAX_IV_LENGTH], *piv = NULL;
	unsigned char *tkey = NULL;
	size_t tkeylen = 0;

	int ok = 0;

	int enc, keep_key = 0;

	enc = ec->cipher ? 1 : 0;

	b = BIO_new(BIO_f_cipher());
	if (!b)
		{
		CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
							ERR_R_MALLOC_FAILURE);
		return NULL;
		}

	BIO_get_cipher_ctx(b, &ctx);

	if (enc)
		{
		ciph = ec->cipher;
		/* If not keeping key set cipher to NULL so subsequent calls
		 * decrypt.
		 */
		if (ec->key)
			ec->cipher = NULL;
		}
	else
		{
		ciph = EVP_get_cipherbyobj(calg->algorithm);

		if (!ciph)
			{
			CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
							CMS_R_UNKNOWN_CIPHER);
			goto err;
			}
		}

	if (EVP_CipherInit_ex(ctx, ciph, NULL, NULL, NULL, enc) <= 0)
		{
		CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
				CMS_R_CIPHER_INITIALISATION_ERROR);
		goto err;
		}

	if (enc)
		{
		int ivlen;
		calg->algorithm = OBJ_nid2obj(EVP_CIPHER_CTX_type(ctx));
		/* Generate a random IV if we need one */
		ivlen = EVP_CIPHER_CTX_iv_length(ctx);
		if (ivlen > 0)
			{
			if (RAND_pseudo_bytes(iv, ivlen) <= 0)
				goto err;
			piv = iv;
			}
		}
	else if (EVP_CIPHER_asn1_to_param(ctx, calg->parameter) <= 0)
		{
		CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
				CMS_R_CIPHER_PARAMETER_INITIALISATION_ERROR);
		goto err;
		}
	tkeylen = EVP_CIPHER_CTX_key_length(ctx);
	/* Generate random session key */
	if (!enc || !ec->key)
		{
		tkey = OPENSSL_malloc(tkeylen);
		if (!tkey)
			{
			CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
							ERR_R_MALLOC_FAILURE);
			goto err;
			}
		if (EVP_CIPHER_CTX_rand_key(ctx, tkey) <= 0)
			goto err;
		}

	if (!ec->key)
		{
		ec->key = tkey;
		ec->keylen = tkeylen;
		tkey = NULL;
		if (enc)
			keep_key = 1;
		else
			ERR_clear_error();
		
		}

	if (ec->keylen != tkeylen)
		{
		/* If necessary set key length */
		if (EVP_CIPHER_CTX_set_key_length(ctx, ec->keylen) <= 0)
			{
			/* Only reveal failure if debugging so we don't
			 * leak information which may be useful in MMA.
			 */
			if (enc || ec->debug)
				{
				CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
						CMS_R_INVALID_KEY_LENGTH);
				goto err;
				}
			else
				{
				/* Use random key */
				OPENSSL_cleanse(ec->key, ec->keylen);
				OPENSSL_free(ec->key);
				ec->key = tkey;
				ec->keylen = tkeylen;
				tkey = NULL;
				ERR_clear_error();
				}
			}
		}

	if (EVP_CipherInit_ex(ctx, NULL, NULL, ec->key, piv, enc) <= 0)
		{
		CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
				CMS_R_CIPHER_INITIALISATION_ERROR);
		goto err;
		}

	if (piv)
		{
		calg->parameter = ASN1_TYPE_new();
		if (!calg->parameter)
			{
			CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
							ERR_R_MALLOC_FAILURE);
			goto err;
			}
		if (EVP_CIPHER_param_to_asn1(ctx, calg->parameter) <= 0)
			{
			CMSerr(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO,
				CMS_R_CIPHER_PARAMETER_INITIALISATION_ERROR);
			goto err;
			}
		}
	ok = 1;

	err:
	if (ec->key && !keep_key)
		{
		OPENSSL_cleanse(ec->key, ec->keylen);
		OPENSSL_free(ec->key);
		ec->key = NULL;
		}
	if (tkey)
		{
		OPENSSL_cleanse(tkey, tkeylen);
		OPENSSL_free(tkey);
		}
	if (ok)
		return b;
	BIO_free(b);
	return NULL;
	}

int cms_EncryptedContent_init(CMS_EncryptedContentInfo *ec, 
				const EVP_CIPHER *cipher,
				const unsigned char *key, size_t keylen)
	{
	ec->cipher = cipher;
	if (key)
		{
		ec->key = OPENSSL_malloc(keylen);
		if (!ec->key)
			return 0;
		memcpy(ec->key, key, keylen);
		}
	ec->keylen = keylen;
	if (cipher)
		ec->contentType = OBJ_nid2obj(NID_pkcs7_data);
	return 1;
	}

int CMS_EncryptedData_set1_key(CMS_ContentInfo *cms, const EVP_CIPHER *ciph,
				const unsigned char *key, size_t keylen)
	{
	CMS_EncryptedContentInfo *ec;
	if (!key || !keylen)
		{
		CMSerr(CMS_F_CMS_ENCRYPTEDDATA_SET1_KEY, CMS_R_NO_KEY);
		return 0;
		}
	if (ciph)
		{
		cms->d.encryptedData = M_ASN1_new_of(CMS_EncryptedData);
		if (!cms->d.encryptedData)
			{
			CMSerr(CMS_F_CMS_ENCRYPTEDDATA_SET1_KEY,
				ERR_R_MALLOC_FAILURE);
			return 0;
			}
		cms->contentType = OBJ_nid2obj(NID_pkcs7_encrypted);
		cms->d.encryptedData->version = 0;
		}
	else if (OBJ_obj2nid(cms->contentType) != NID_pkcs7_encrypted)
		{
		CMSerr(CMS_F_CMS_ENCRYPTEDDATA_SET1_KEY,
						CMS_R_NOT_ENCRYPTED_DATA);
		return 0;
		}
	ec = cms->d.encryptedData->encryptedContentInfo;
	return cms_EncryptedContent_init(ec, ciph, key, keylen);
	}

BIO *cms_EncryptedData_init_bio(CMS_ContentInfo *cms)
	{
	CMS_EncryptedData *enc = cms->d.encryptedData;
	if (enc->encryptedContentInfo->cipher && enc->unprotectedAttrs)
		enc->version = 2;
	return cms_EncryptedContent_init_bio(enc->encryptedContentInfo);
	}