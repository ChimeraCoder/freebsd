
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

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pkcs7.h>
#include <openssl/pem.h>
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#ifndef OPENSSL_NO_DSA
#include <openssl/dsa.h>
#endif
#ifndef OPENSSL_NO_DH
#include <openssl/dh.h>
#endif

#ifndef OPENSSL_NO_RSA
static RSA *pkey_get_rsa(EVP_PKEY *key, RSA **rsa);
#endif
#ifndef OPENSSL_NO_DSA
static DSA *pkey_get_dsa(EVP_PKEY *key, DSA **dsa);
#endif

#ifndef OPENSSL_NO_EC
static EC_KEY *pkey_get_eckey(EVP_PKEY *key, EC_KEY **eckey);
#endif

IMPLEMENT_PEM_rw(X509_REQ, X509_REQ, PEM_STRING_X509_REQ, X509_REQ)

IMPLEMENT_PEM_write(X509_REQ_NEW, X509_REQ, PEM_STRING_X509_REQ_OLD, X509_REQ)

IMPLEMENT_PEM_rw(X509_CRL, X509_CRL, PEM_STRING_X509_CRL, X509_CRL)

IMPLEMENT_PEM_rw(PKCS7, PKCS7, PEM_STRING_PKCS7, PKCS7)

IMPLEMENT_PEM_rw(NETSCAPE_CERT_SEQUENCE, NETSCAPE_CERT_SEQUENCE,
					PEM_STRING_X509, NETSCAPE_CERT_SEQUENCE)


#ifndef OPENSSL_NO_RSA

/* We treat RSA or DSA private keys as a special case.
 *
 * For private keys we read in an EVP_PKEY structure with
 * PEM_read_bio_PrivateKey() and extract the relevant private
 * key: this means can handle "traditional" and PKCS#8 formats
 * transparently.
 */

static RSA *pkey_get_rsa(EVP_PKEY *key, RSA **rsa)
{
	RSA *rtmp;
	if(!key) return NULL;
	rtmp = EVP_PKEY_get1_RSA(key);
	EVP_PKEY_free(key);
	if(!rtmp) return NULL;
	if(rsa) {
		RSA_free(*rsa);
		*rsa = rtmp;
	}
	return rtmp;
}

RSA *PEM_read_bio_RSAPrivateKey(BIO *bp, RSA **rsa, pem_password_cb *cb,
								void *u)
{
	EVP_PKEY *pktmp;
	pktmp = PEM_read_bio_PrivateKey(bp, NULL, cb, u);
	return pkey_get_rsa(pktmp, rsa);
}

#ifndef OPENSSL_NO_FP_API

RSA *PEM_read_RSAPrivateKey(FILE *fp, RSA **rsa, pem_password_cb *cb,
								void *u)
{
	EVP_PKEY *pktmp;
	pktmp = PEM_read_PrivateKey(fp, NULL, cb, u);
	return pkey_get_rsa(pktmp, rsa);
}

#endif

#ifdef OPENSSL_FIPS

int PEM_write_bio_RSAPrivateKey(BIO *bp, RSA *x, const EVP_CIPHER *enc,
                                               unsigned char *kstr, int klen,
                                               pem_password_cb *cb, void *u)
{
	if (FIPS_mode())
		{
		EVP_PKEY *k;
		int ret;
		k = EVP_PKEY_new();
		if (!k)
			return 0;
		EVP_PKEY_set1_RSA(k, x);

		ret = PEM_write_bio_PrivateKey(bp, k, enc, kstr, klen, cb, u);
		EVP_PKEY_free(k);
		return ret;
		}
	else
		return PEM_ASN1_write_bio((i2d_of_void *)i2d_RSAPrivateKey,
					PEM_STRING_RSA,bp,x,enc,kstr,klen,cb,u);
}

#ifndef OPENSSL_NO_FP_API
int PEM_write_RSAPrivateKey(FILE *fp, RSA *x, const EVP_CIPHER *enc,
                                               unsigned char *kstr, int klen,
                                               pem_password_cb *cb, void *u)
{
	if (FIPS_mode())
		{
		EVP_PKEY *k;
		int ret;
		k = EVP_PKEY_new();
		if (!k)
			return 0;

		EVP_PKEY_set1_RSA(k, x);

		ret = PEM_write_PrivateKey(fp, k, enc, kstr, klen, cb, u);
		EVP_PKEY_free(k);
		return ret;
		}
	else
		return PEM_ASN1_write((i2d_of_void *)i2d_RSAPrivateKey,
					PEM_STRING_RSA,fp,x,enc,kstr,klen,cb,u);
}
#endif

#else

IMPLEMENT_PEM_write_cb_const(RSAPrivateKey, RSA, PEM_STRING_RSA, RSAPrivateKey)

#endif

IMPLEMENT_PEM_rw_const(RSAPublicKey, RSA, PEM_STRING_RSA_PUBLIC, RSAPublicKey)
IMPLEMENT_PEM_rw(RSA_PUBKEY, RSA, PEM_STRING_PUBLIC, RSA_PUBKEY)

#endif

#ifndef OPENSSL_NO_DSA

static DSA *pkey_get_dsa(EVP_PKEY *key, DSA **dsa)
{
	DSA *dtmp;
	if(!key) return NULL;
	dtmp = EVP_PKEY_get1_DSA(key);
	EVP_PKEY_free(key);
	if(!dtmp) return NULL;
	if(dsa) {
		DSA_free(*dsa);
		*dsa = dtmp;
	}
	return dtmp;
}

DSA *PEM_read_bio_DSAPrivateKey(BIO *bp, DSA **dsa, pem_password_cb *cb,
								void *u)
{
	EVP_PKEY *pktmp;
	pktmp = PEM_read_bio_PrivateKey(bp, NULL, cb, u);
	return pkey_get_dsa(pktmp, dsa);	/* will free pktmp */
}

#ifdef OPENSSL_FIPS

int PEM_write_bio_DSAPrivateKey(BIO *bp, DSA *x, const EVP_CIPHER *enc,
                                               unsigned char *kstr, int klen,
                                               pem_password_cb *cb, void *u)
{
	if (FIPS_mode())
		{
		EVP_PKEY *k;
		int ret;
		k = EVP_PKEY_new();
		if (!k)
			return 0;
		EVP_PKEY_set1_DSA(k, x);

		ret = PEM_write_bio_PrivateKey(bp, k, enc, kstr, klen, cb, u);
		EVP_PKEY_free(k);
		return ret;
		}
	else
		return PEM_ASN1_write_bio((i2d_of_void *)i2d_DSAPrivateKey,
					PEM_STRING_DSA,bp,x,enc,kstr,klen,cb,u);
}

#ifndef OPENSSL_NO_FP_API
int PEM_write_DSAPrivateKey(FILE *fp, DSA *x, const EVP_CIPHER *enc,
                                               unsigned char *kstr, int klen,
                                               pem_password_cb *cb, void *u)
{
	if (FIPS_mode())
		{
		EVP_PKEY *k;
		int ret;
		k = EVP_PKEY_new();
		if (!k)
			return 0;
		EVP_PKEY_set1_DSA(k, x);
		ret = PEM_write_PrivateKey(fp, k, enc, kstr, klen, cb, u);
		EVP_PKEY_free(k);
		return ret;
		}
	else
		return PEM_ASN1_write((i2d_of_void *)i2d_DSAPrivateKey,
					PEM_STRING_DSA,fp,x,enc,kstr,klen,cb,u);
}
#endif

#else

IMPLEMENT_PEM_write_cb_const(DSAPrivateKey, DSA, PEM_STRING_DSA, DSAPrivateKey)

#endif

IMPLEMENT_PEM_rw(DSA_PUBKEY, DSA, PEM_STRING_PUBLIC, DSA_PUBKEY)

#ifndef OPENSSL_NO_FP_API

DSA *PEM_read_DSAPrivateKey(FILE *fp, DSA **dsa, pem_password_cb *cb,
								void *u)
{
	EVP_PKEY *pktmp;
	pktmp = PEM_read_PrivateKey(fp, NULL, cb, u);
	return pkey_get_dsa(pktmp, dsa);	/* will free pktmp */
}

#endif

IMPLEMENT_PEM_rw_const(DSAparams, DSA, PEM_STRING_DSAPARAMS, DSAparams)

#endif


#ifndef OPENSSL_NO_EC
static EC_KEY *pkey_get_eckey(EVP_PKEY *key, EC_KEY **eckey)
{
	EC_KEY *dtmp;
	if(!key) return NULL;
	dtmp = EVP_PKEY_get1_EC_KEY(key);
	EVP_PKEY_free(key);
	if(!dtmp) return NULL;
	if(eckey) 
	{
 		EC_KEY_free(*eckey);
		*eckey = dtmp;
	}
	return dtmp;
}

EC_KEY *PEM_read_bio_ECPrivateKey(BIO *bp, EC_KEY **key, pem_password_cb *cb,
							void *u)
{
	EVP_PKEY *pktmp;
	pktmp = PEM_read_bio_PrivateKey(bp, NULL, cb, u);
	return pkey_get_eckey(pktmp, key);	/* will free pktmp */
}

IMPLEMENT_PEM_rw_const(ECPKParameters, EC_GROUP, PEM_STRING_ECPARAMETERS, ECPKParameters)



#ifdef OPENSSL_FIPS

int PEM_write_bio_ECPrivateKey(BIO *bp, EC_KEY *x, const EVP_CIPHER *enc,
                                               unsigned char *kstr, int klen,
                                               pem_password_cb *cb, void *u)
{
	if (FIPS_mode())
		{
		EVP_PKEY *k;
		int ret;
		k = EVP_PKEY_new();
		if (!k)
			return 0;
		EVP_PKEY_set1_EC_KEY(k, x);

		ret = PEM_write_bio_PrivateKey(bp, k, enc, kstr, klen, cb, u);
		EVP_PKEY_free(k);
		return ret;
		}
	else
		return PEM_ASN1_write_bio((i2d_of_void *)i2d_ECPrivateKey,
						PEM_STRING_ECPRIVATEKEY,
						bp,x,enc,kstr,klen,cb,u);
}

#ifndef OPENSSL_NO_FP_API
int PEM_write_ECPrivateKey(FILE *fp, EC_KEY *x, const EVP_CIPHER *enc,
                                               unsigned char *kstr, int klen,
                                               pem_password_cb *cb, void *u)
{
	if (FIPS_mode())
		{
		EVP_PKEY *k;
		int ret;
		k = EVP_PKEY_new();
		if (!k)
			return 0;
		EVP_PKEY_set1_EC_KEY(k, x);
		ret = PEM_write_PrivateKey(fp, k, enc, kstr, klen, cb, u);
		EVP_PKEY_free(k);
		return ret;
		}
	else
		return PEM_ASN1_write((i2d_of_void *)i2d_ECPrivateKey,
						PEM_STRING_ECPRIVATEKEY,
						fp,x,enc,kstr,klen,cb,u);
}
#endif

#else

IMPLEMENT_PEM_write_cb(ECPrivateKey, EC_KEY, PEM_STRING_ECPRIVATEKEY, ECPrivateKey)

#endif

IMPLEMENT_PEM_rw(EC_PUBKEY, EC_KEY, PEM_STRING_PUBLIC, EC_PUBKEY)

#ifndef OPENSSL_NO_FP_API
 
EC_KEY *PEM_read_ECPrivateKey(FILE *fp, EC_KEY **eckey, pem_password_cb *cb,
 								void *u)
{
	EVP_PKEY *pktmp;
	pktmp = PEM_read_PrivateKey(fp, NULL, cb, u);
	return pkey_get_eckey(pktmp, eckey);	/* will free pktmp */
}

#endif

#endif

#ifndef OPENSSL_NO_DH

IMPLEMENT_PEM_rw_const(DHparams, DH, PEM_STRING_DHPARAMS, DHparams)

#endif

IMPLEMENT_PEM_rw(PUBKEY, EVP_PKEY, PEM_STRING_PUBLIC, PUBKEY)