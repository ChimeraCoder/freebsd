
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
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include "asn1_locl.h"
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#ifndef OPENSSL_NO_DSA
#include <openssl/dsa.h>
#endif

/* Minor tweak to operation: free up EVP_PKEY */
static int pubkey_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
			void *exarg)
	{
	if (operation == ASN1_OP_FREE_POST)
		{
		X509_PUBKEY *pubkey = (X509_PUBKEY *)*pval;
		EVP_PKEY_free(pubkey->pkey);
		}
	return 1;
	}

ASN1_SEQUENCE_cb(X509_PUBKEY, pubkey_cb) = {
	ASN1_SIMPLE(X509_PUBKEY, algor, X509_ALGOR),
	ASN1_SIMPLE(X509_PUBKEY, public_key, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END_cb(X509_PUBKEY, X509_PUBKEY)

IMPLEMENT_ASN1_FUNCTIONS(X509_PUBKEY)

int X509_PUBKEY_set(X509_PUBKEY **x, EVP_PKEY *pkey)
	{
	X509_PUBKEY *pk=NULL;

	if (x == NULL) return(0);

	if ((pk=X509_PUBKEY_new()) == NULL) goto error;

	if (pkey->ameth)
		{
		if (pkey->ameth->pub_encode)
			{
			if (!pkey->ameth->pub_encode(pk, pkey))
				{
				X509err(X509_F_X509_PUBKEY_SET,
					X509_R_PUBLIC_KEY_ENCODE_ERROR);
				goto error;
				}
			}
		else
			{
			X509err(X509_F_X509_PUBKEY_SET,
				X509_R_METHOD_NOT_SUPPORTED);
			goto error;
			}
		}
	else
		{
		X509err(X509_F_X509_PUBKEY_SET,X509_R_UNSUPPORTED_ALGORITHM);
		goto error;
		}

	if (*x != NULL)
		X509_PUBKEY_free(*x);

	*x=pk;

	return 1;
error:
	if (pk != NULL) X509_PUBKEY_free(pk);
	return 0;
	}

EVP_PKEY *X509_PUBKEY_get(X509_PUBKEY *key)
	{
	EVP_PKEY *ret=NULL;

	if (key == NULL) goto error;

	if (key->pkey != NULL)
		{
		CRYPTO_add(&key->pkey->references, 1, CRYPTO_LOCK_EVP_PKEY);
		return key->pkey;
		}

	if (key->public_key == NULL) goto error;

	if ((ret = EVP_PKEY_new()) == NULL)
		{
		X509err(X509_F_X509_PUBKEY_GET, ERR_R_MALLOC_FAILURE);
		goto error;
		}

	if (!EVP_PKEY_set_type(ret, OBJ_obj2nid(key->algor->algorithm)))
		{
		X509err(X509_F_X509_PUBKEY_GET,X509_R_UNSUPPORTED_ALGORITHM);
		goto error;
		}

	if (ret->ameth->pub_decode)
		{
		if (!ret->ameth->pub_decode(ret, key))
			{
			X509err(X509_F_X509_PUBKEY_GET,
						X509_R_PUBLIC_KEY_DECODE_ERROR);
			goto error;
			}
		}
	else
		{
		X509err(X509_F_X509_PUBKEY_GET, X509_R_METHOD_NOT_SUPPORTED);
		goto error;
		}

	/* Check to see if another thread set key->pkey first */
	CRYPTO_w_lock(CRYPTO_LOCK_EVP_PKEY);
	if (key->pkey)
		{
		CRYPTO_w_unlock(CRYPTO_LOCK_EVP_PKEY);
		EVP_PKEY_free(ret);
		ret = key->pkey;
		}
	else
		{
		key->pkey = ret;
		CRYPTO_w_unlock(CRYPTO_LOCK_EVP_PKEY);
		}
	CRYPTO_add(&ret->references, 1, CRYPTO_LOCK_EVP_PKEY);

	return ret;

	error:
	if (ret != NULL)
		EVP_PKEY_free(ret);
	return(NULL);
	}

/* Now two pseudo ASN1 routines that take an EVP_PKEY structure
 * and encode or decode as X509_PUBKEY
 */

EVP_PKEY *d2i_PUBKEY(EVP_PKEY **a, const unsigned char **pp,
	     long length)
	{
	X509_PUBKEY *xpk;
	EVP_PKEY *pktmp;
	xpk = d2i_X509_PUBKEY(NULL, pp, length);
	if(!xpk) return NULL;
	pktmp = X509_PUBKEY_get(xpk);
	X509_PUBKEY_free(xpk);
	if(!pktmp) return NULL;
	if(a)
		{
		EVP_PKEY_free(*a);
		*a = pktmp;
		}
	return pktmp;
	}

int i2d_PUBKEY(EVP_PKEY *a, unsigned char **pp)
	{
	X509_PUBKEY *xpk=NULL;
	int ret;
	if(!a) return 0;
	if(!X509_PUBKEY_set(&xpk, a)) return 0;
	ret = i2d_X509_PUBKEY(xpk, pp);
	X509_PUBKEY_free(xpk);
	return ret;
	}

/* The following are equivalents but which return RSA and DSA
 * keys
 */
#ifndef OPENSSL_NO_RSA
RSA *d2i_RSA_PUBKEY(RSA **a, const unsigned char **pp,
	     long length)
	{
	EVP_PKEY *pkey;
	RSA *key;
	const unsigned char *q;
	q = *pp;
	pkey = d2i_PUBKEY(NULL, &q, length);
	if (!pkey) return NULL;
	key = EVP_PKEY_get1_RSA(pkey);
	EVP_PKEY_free(pkey);
	if (!key) return NULL;
	*pp = q;
	if (a)
		{
		RSA_free(*a);
		*a = key;
		}
	return key;
	}

int i2d_RSA_PUBKEY(RSA *a, unsigned char **pp)
	{
	EVP_PKEY *pktmp;
	int ret;
	if (!a) return 0;
	pktmp = EVP_PKEY_new();
	if (!pktmp)
		{
		ASN1err(ASN1_F_I2D_RSA_PUBKEY, ERR_R_MALLOC_FAILURE);
		return 0;
		}
	EVP_PKEY_set1_RSA(pktmp, a);
	ret = i2d_PUBKEY(pktmp, pp);
	EVP_PKEY_free(pktmp);
	return ret;
	}
#endif

#ifndef OPENSSL_NO_DSA
DSA *d2i_DSA_PUBKEY(DSA **a, const unsigned char **pp,
	     long length)
	{
	EVP_PKEY *pkey;
	DSA *key;
	const unsigned char *q;
	q = *pp;
	pkey = d2i_PUBKEY(NULL, &q, length);
	if (!pkey) return NULL;
	key = EVP_PKEY_get1_DSA(pkey);
	EVP_PKEY_free(pkey);
	if (!key) return NULL;
	*pp = q;
	if (a)
		{
		DSA_free(*a);
		*a = key;
		}
	return key;
	}

int i2d_DSA_PUBKEY(DSA *a, unsigned char **pp)
	{
	EVP_PKEY *pktmp;
	int ret;
	if(!a) return 0;
	pktmp = EVP_PKEY_new();
	if(!pktmp)
		{
		ASN1err(ASN1_F_I2D_DSA_PUBKEY, ERR_R_MALLOC_FAILURE);
		return 0;
		}
	EVP_PKEY_set1_DSA(pktmp, a);
	ret = i2d_PUBKEY(pktmp, pp);
	EVP_PKEY_free(pktmp);
	return ret;
	}
#endif

#ifndef OPENSSL_NO_EC
EC_KEY *d2i_EC_PUBKEY(EC_KEY **a, const unsigned char **pp, long length)
	{
	EVP_PKEY *pkey;
	EC_KEY *key;
	const unsigned char *q;
	q = *pp;
	pkey = d2i_PUBKEY(NULL, &q, length);
	if (!pkey) return(NULL);
	key = EVP_PKEY_get1_EC_KEY(pkey);
	EVP_PKEY_free(pkey);
	if (!key)  return(NULL);
	*pp = q;
	if (a)
		{
		EC_KEY_free(*a);
		*a = key;
		}
	return(key);
	}

int i2d_EC_PUBKEY(EC_KEY *a, unsigned char **pp)
	{
	EVP_PKEY *pktmp;
	int ret;
	if (!a)	return(0);
	if ((pktmp = EVP_PKEY_new()) == NULL)
		{
		ASN1err(ASN1_F_I2D_EC_PUBKEY, ERR_R_MALLOC_FAILURE);
		return(0);
		}
	EVP_PKEY_set1_EC_KEY(pktmp, a);
	ret = i2d_PUBKEY(pktmp, pp);
	EVP_PKEY_free(pktmp);
	return(ret);
	}
#endif

int X509_PUBKEY_set0_param(X509_PUBKEY *pub, ASN1_OBJECT *aobj,
					int ptype, void *pval,
					unsigned char *penc, int penclen)
	{
	if (!X509_ALGOR_set0(pub->algor, aobj, ptype, pval))
		return 0;
	if (penc)
		{
		if (pub->public_key->data)
			OPENSSL_free(pub->public_key->data);
		pub->public_key->data = penc;
		pub->public_key->length = penclen;
  		/* Set number of unused bits to zero */
		pub->public_key->flags&= ~(ASN1_STRING_FLAG_BITS_LEFT|0x07);
		pub->public_key->flags|=ASN1_STRING_FLAG_BITS_LEFT;
		}
	return 1;
	}

int X509_PUBKEY_get0_param(ASN1_OBJECT **ppkalg,
		const unsigned char **pk, int *ppklen,
		X509_ALGOR **pa,
		X509_PUBKEY *pub)
	{
	if (ppkalg)
		*ppkalg = pub->algor->algorithm;
	if (pk)
		{
		*pk = pub->public_key->data;
		*ppklen = pub->public_key->length;
		}
	if (pa)
		*pa = pub->algor;
	return 1;
	}