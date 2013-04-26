
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
#include <ctype.h>
#include "cryptlib.h"
#include <openssl/asn1.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

int X509_issuer_and_serial_cmp(const X509 *a, const X509 *b)
	{
	int i;
	X509_CINF *ai,*bi;

	ai=a->cert_info;
	bi=b->cert_info;
	i=M_ASN1_INTEGER_cmp(ai->serialNumber,bi->serialNumber);
	if (i) return(i);
	return(X509_NAME_cmp(ai->issuer,bi->issuer));
	}

#ifndef OPENSSL_NO_MD5
unsigned long X509_issuer_and_serial_hash(X509 *a)
	{
	unsigned long ret=0;
	EVP_MD_CTX ctx;
	unsigned char md[16];
	char *f;

	EVP_MD_CTX_init(&ctx);
	f=X509_NAME_oneline(a->cert_info->issuer,NULL,0);
	if (!EVP_DigestInit_ex(&ctx, EVP_md5(), NULL))
		goto err;
	if (!EVP_DigestUpdate(&ctx,(unsigned char *)f,strlen(f)))
		goto err;
	OPENSSL_free(f);
	if(!EVP_DigestUpdate(&ctx,(unsigned char *)a->cert_info->serialNumber->data,
		(unsigned long)a->cert_info->serialNumber->length))
		goto err;
	if (!EVP_DigestFinal_ex(&ctx,&(md[0]),NULL))
		goto err;
	ret=(	((unsigned long)md[0]     )|((unsigned long)md[1]<<8L)|
		((unsigned long)md[2]<<16L)|((unsigned long)md[3]<<24L)
		)&0xffffffffL;
	err:
	EVP_MD_CTX_cleanup(&ctx);
	return(ret);
	}
#endif
	
int X509_issuer_name_cmp(const X509 *a, const X509 *b)
	{
	return(X509_NAME_cmp(a->cert_info->issuer,b->cert_info->issuer));
	}

int X509_subject_name_cmp(const X509 *a, const X509 *b)
	{
	return(X509_NAME_cmp(a->cert_info->subject,b->cert_info->subject));
	}

int X509_CRL_cmp(const X509_CRL *a, const X509_CRL *b)
	{
	return(X509_NAME_cmp(a->crl->issuer,b->crl->issuer));
	}

#ifndef OPENSSL_NO_SHA
int X509_CRL_match(const X509_CRL *a, const X509_CRL *b)
	{
	return memcmp(a->sha1_hash, b->sha1_hash, 20);
	}
#endif

X509_NAME *X509_get_issuer_name(X509 *a)
	{
	return(a->cert_info->issuer);
	}

unsigned long X509_issuer_name_hash(X509 *x)
	{
	return(X509_NAME_hash(x->cert_info->issuer));
	}

#ifndef OPENSSL_NO_MD5
unsigned long X509_issuer_name_hash_old(X509 *x)
	{
	return(X509_NAME_hash_old(x->cert_info->issuer));
	}
#endif

X509_NAME *X509_get_subject_name(X509 *a)
	{
	return(a->cert_info->subject);
	}

ASN1_INTEGER *X509_get_serialNumber(X509 *a)
	{
	return(a->cert_info->serialNumber);
	}

unsigned long X509_subject_name_hash(X509 *x)
	{
	return(X509_NAME_hash(x->cert_info->subject));
	}

#ifndef OPENSSL_NO_MD5
unsigned long X509_subject_name_hash_old(X509 *x)
	{
	return(X509_NAME_hash_old(x->cert_info->subject));
	}
#endif

#ifndef OPENSSL_NO_SHA
/* Compare two certificates: they must be identical for
 * this to work. NB: Although "cmp" operations are generally
 * prototyped to take "const" arguments (eg. for use in
 * STACKs), the way X509 handling is - these operations may
 * involve ensuring the hashes are up-to-date and ensuring
 * certain cert information is cached. So this is the point
 * where the "depth-first" constification tree has to halt
 * with an evil cast.
 */
int X509_cmp(const X509 *a, const X509 *b)
{
	/* ensure hash is valid */
	X509_check_purpose((X509 *)a, -1, 0);
	X509_check_purpose((X509 *)b, -1, 0);

	return memcmp(a->sha1_hash, b->sha1_hash, SHA_DIGEST_LENGTH);
}
#endif


int X509_NAME_cmp(const X509_NAME *a, const X509_NAME *b)
	{
	int ret;

	/* Ensure canonical encoding is present and up to date */

	if (!a->canon_enc || a->modified)
		{
		ret = i2d_X509_NAME((X509_NAME *)a, NULL);
		if (ret < 0)
			return -2;
		}

	if (!b->canon_enc || b->modified)
		{
		ret = i2d_X509_NAME((X509_NAME *)b, NULL);
		if (ret < 0)
			return -2;
		}

	ret = a->canon_enclen - b->canon_enclen;

	if (ret)
		return ret;

	return memcmp(a->canon_enc, b->canon_enc, a->canon_enclen);

	}

unsigned long X509_NAME_hash(X509_NAME *x)
	{
	unsigned long ret=0;
	unsigned char md[SHA_DIGEST_LENGTH];

	/* Make sure X509_NAME structure contains valid cached encoding */
	i2d_X509_NAME(x,NULL);
	if (!EVP_Digest(x->canon_enc, x->canon_enclen, md, NULL, EVP_sha1(),
		NULL))
		return 0;

	ret=(	((unsigned long)md[0]     )|((unsigned long)md[1]<<8L)|
		((unsigned long)md[2]<<16L)|((unsigned long)md[3]<<24L)
		)&0xffffffffL;
	return(ret);
	}


#ifndef OPENSSL_NO_MD5
/* I now DER encode the name and hash it.  Since I cache the DER encoding,
 * this is reasonably efficient. */

unsigned long X509_NAME_hash_old(X509_NAME *x)
	{
	EVP_MD_CTX md_ctx;
	unsigned long ret=0;
	unsigned char md[16];

	/* Make sure X509_NAME structure contains valid cached encoding */
	i2d_X509_NAME(x,NULL);
	EVP_MD_CTX_init(&md_ctx);
	EVP_MD_CTX_set_flags(&md_ctx, EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
	if (EVP_DigestInit_ex(&md_ctx, EVP_md5(), NULL)
	    && EVP_DigestUpdate(&md_ctx, x->bytes->data, x->bytes->length)
	    && EVP_DigestFinal_ex(&md_ctx,md,NULL))
		ret=(((unsigned long)md[0]     )|((unsigned long)md[1]<<8L)|
		     ((unsigned long)md[2]<<16L)|((unsigned long)md[3]<<24L)
		     )&0xffffffffL;
	EVP_MD_CTX_cleanup(&md_ctx);

	return(ret);
	}
#endif

/* Search a stack of X509 for a match */
X509 *X509_find_by_issuer_and_serial(STACK_OF(X509) *sk, X509_NAME *name,
		ASN1_INTEGER *serial)
	{
	int i;
	X509_CINF cinf;
	X509 x,*x509=NULL;

	if(!sk) return NULL;

	x.cert_info= &cinf;
	cinf.serialNumber=serial;
	cinf.issuer=name;

	for (i=0; i<sk_X509_num(sk); i++)
		{
		x509=sk_X509_value(sk,i);
		if (X509_issuer_and_serial_cmp(x509,&x) == 0)
			return(x509);
		}
	return(NULL);
	}

X509 *X509_find_by_subject(STACK_OF(X509) *sk, X509_NAME *name)
	{
	X509 *x509;
	int i;

	for (i=0; i<sk_X509_num(sk); i++)
		{
		x509=sk_X509_value(sk,i);
		if (X509_NAME_cmp(X509_get_subject_name(x509),name) == 0)
			return(x509);
		}
	return(NULL);
	}

EVP_PKEY *X509_get_pubkey(X509 *x)
	{
	if ((x == NULL) || (x->cert_info == NULL))
		return(NULL);
	return(X509_PUBKEY_get(x->cert_info->key));
	}

ASN1_BIT_STRING *X509_get0_pubkey_bitstr(const X509 *x)
	{
	if(!x) return NULL;
	return x->cert_info->key->public_key;
	}

int X509_check_private_key(X509 *x, EVP_PKEY *k)
	{
	EVP_PKEY *xk;
	int ret;

	xk=X509_get_pubkey(x);

	if (xk)
		ret = EVP_PKEY_cmp(xk, k);
	else
		ret = -2;

	switch (ret)
		{
	case 1:
		break;
	case 0:
		X509err(X509_F_X509_CHECK_PRIVATE_KEY,X509_R_KEY_VALUES_MISMATCH);
		break;
	case -1:
		X509err(X509_F_X509_CHECK_PRIVATE_KEY,X509_R_KEY_TYPE_MISMATCH);
		break;
	case -2:
	        X509err(X509_F_X509_CHECK_PRIVATE_KEY,X509_R_UNKNOWN_KEY_TYPE);
		}
	if (xk)
		EVP_PKEY_free(xk);
	if (ret > 0)
		return 1;
	return 0;
	}