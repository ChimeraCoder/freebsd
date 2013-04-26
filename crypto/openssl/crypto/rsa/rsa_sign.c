
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
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include "rsa_locl.h"

/* Size of an SSL signature: MD5+SHA1 */
#define SSL_SIG_LENGTH	36

int RSA_sign(int type, const unsigned char *m, unsigned int m_len,
	     unsigned char *sigret, unsigned int *siglen, RSA *rsa)
	{
	X509_SIG sig;
	ASN1_TYPE parameter;
	int i,j,ret=1;
	unsigned char *p, *tmps = NULL;
	const unsigned char *s = NULL;
	X509_ALGOR algor;
	ASN1_OCTET_STRING digest;
#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !(rsa->meth->flags & RSA_FLAG_FIPS_METHOD)
			&& !(rsa->flags & RSA_FLAG_NON_FIPS_ALLOW))
		{
		RSAerr(RSA_F_RSA_SIGN, RSA_R_NON_FIPS_RSA_METHOD);
		return 0;
		}
#endif
	if((rsa->flags & RSA_FLAG_SIGN_VER) && rsa->meth->rsa_sign)
		{
		return rsa->meth->rsa_sign(type, m, m_len,
			sigret, siglen, rsa);
		}
	/* Special case: SSL signature, just check the length */
	if(type == NID_md5_sha1) {
		if(m_len != SSL_SIG_LENGTH) {
			RSAerr(RSA_F_RSA_SIGN,RSA_R_INVALID_MESSAGE_LENGTH);
			return(0);
		}
		i = SSL_SIG_LENGTH;
		s = m;
	} else {
		sig.algor= &algor;
		sig.algor->algorithm=OBJ_nid2obj(type);
		if (sig.algor->algorithm == NULL)
			{
			RSAerr(RSA_F_RSA_SIGN,RSA_R_UNKNOWN_ALGORITHM_TYPE);
			return(0);
			}
		if (sig.algor->algorithm->length == 0)
			{
			RSAerr(RSA_F_RSA_SIGN,RSA_R_THE_ASN1_OBJECT_IDENTIFIER_IS_NOT_KNOWN_FOR_THIS_MD);
			return(0);
			}
		parameter.type=V_ASN1_NULL;
		parameter.value.ptr=NULL;
		sig.algor->parameter= &parameter;

		sig.digest= &digest;
		sig.digest->data=(unsigned char *)m; /* TMP UGLY CAST */
		sig.digest->length=m_len;

		i=i2d_X509_SIG(&sig,NULL);
	}
	j=RSA_size(rsa);
	if (i > (j-RSA_PKCS1_PADDING_SIZE))
		{
		RSAerr(RSA_F_RSA_SIGN,RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY);
		return(0);
		}
	if(type != NID_md5_sha1) {
		tmps=(unsigned char *)OPENSSL_malloc((unsigned int)j+1);
		if (tmps == NULL)
			{
			RSAerr(RSA_F_RSA_SIGN,ERR_R_MALLOC_FAILURE);
			return(0);
			}
		p=tmps;
		i2d_X509_SIG(&sig,&p);
		s=tmps;
	}
	i=RSA_private_encrypt(i,s,sigret,rsa,RSA_PKCS1_PADDING);
	if (i <= 0)
		ret=0;
	else
		*siglen=i;

	if(type != NID_md5_sha1) {
		OPENSSL_cleanse(tmps,(unsigned int)j+1);
		OPENSSL_free(tmps);
	}
	return(ret);
	}

int int_rsa_verify(int dtype, const unsigned char *m,
			  unsigned int m_len,
			  unsigned char *rm, size_t *prm_len,
			  const unsigned char *sigbuf, size_t siglen,
			  RSA *rsa)
	{
	int i,ret=0,sigtype;
	unsigned char *s;
	X509_SIG *sig=NULL;

#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !(rsa->meth->flags & RSA_FLAG_FIPS_METHOD)
			&& !(rsa->flags & RSA_FLAG_NON_FIPS_ALLOW))
		{
		RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_NON_FIPS_RSA_METHOD);
		return 0;
		}
#endif

	if (siglen != (unsigned int)RSA_size(rsa))
		{
		RSAerr(RSA_F_INT_RSA_VERIFY,RSA_R_WRONG_SIGNATURE_LENGTH);
		return(0);
		}

	if((dtype == NID_md5_sha1) && rm)
		{
		i = RSA_public_decrypt((int)siglen,
					sigbuf,rm,rsa,RSA_PKCS1_PADDING);
		if (i <= 0)
			return 0;
		*prm_len = i;
		return 1;
		}

	s=(unsigned char *)OPENSSL_malloc((unsigned int)siglen);
	if (s == NULL)
		{
		RSAerr(RSA_F_INT_RSA_VERIFY,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	if((dtype == NID_md5_sha1) && (m_len != SSL_SIG_LENGTH) ) {
			RSAerr(RSA_F_INT_RSA_VERIFY,RSA_R_INVALID_MESSAGE_LENGTH);
			goto err;
	}
	i=RSA_public_decrypt((int)siglen,sigbuf,s,rsa,RSA_PKCS1_PADDING);

	if (i <= 0) goto err;
	/* Oddball MDC2 case: signature can be OCTET STRING.
	 * check for correct tag and length octets.
	 */
	if (dtype == NID_mdc2 && i == 18 && s[0] == 0x04 && s[1] == 0x10)
		{
		if (rm)
			{
			memcpy(rm, s + 2, 16);
			*prm_len = 16;
			ret = 1;
			}
		else if(memcmp(m, s + 2, 16))
			RSAerr(RSA_F_INT_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
		else
			ret = 1;
		}

	/* Special case: SSL signature */
	if(dtype == NID_md5_sha1) {
		if((i != SSL_SIG_LENGTH) || memcmp(s, m, SSL_SIG_LENGTH))
				RSAerr(RSA_F_INT_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
		else ret = 1;
	} else {
		const unsigned char *p=s;
		sig=d2i_X509_SIG(NULL,&p,(long)i);

		if (sig == NULL) goto err;

		/* Excess data can be used to create forgeries */
		if(p != s+i)
			{
			RSAerr(RSA_F_INT_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
			goto err;
			}

		/* Parameters to the signature algorithm can also be used to
		   create forgeries */
		if(sig->algor->parameter
		   && ASN1_TYPE_get(sig->algor->parameter) != V_ASN1_NULL)
			{
			RSAerr(RSA_F_INT_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
			goto err;
			}

		sigtype=OBJ_obj2nid(sig->algor->algorithm);


	#ifdef RSA_DEBUG
		/* put a backward compatibility flag in EAY */
		fprintf(stderr,"in(%s) expect(%s)\n",OBJ_nid2ln(sigtype),
			OBJ_nid2ln(dtype));
	#endif
		if (sigtype != dtype)
			{
			if (((dtype == NID_md5) &&
				(sigtype == NID_md5WithRSAEncryption)) ||
				((dtype == NID_md2) &&
				(sigtype == NID_md2WithRSAEncryption)))
				{
				/* ok, we will let it through */
#if !defined(OPENSSL_NO_STDIO) && !defined(OPENSSL_SYS_WIN16)
				fprintf(stderr,"signature has problems, re-make with post SSLeay045\n");
#endif
				}
			else
				{
				RSAerr(RSA_F_INT_RSA_VERIFY,
						RSA_R_ALGORITHM_MISMATCH);
				goto err;
				}
			}
		if (rm)
			{
			const EVP_MD *md;
			md = EVP_get_digestbynid(dtype);
			if (md && (EVP_MD_size(md) != sig->digest->length))
				RSAerr(RSA_F_INT_RSA_VERIFY,
						RSA_R_INVALID_DIGEST_LENGTH);
			else
				{
				memcpy(rm, sig->digest->data,
							sig->digest->length);
				*prm_len = sig->digest->length;
				ret = 1;
				}
			}
		else if (((unsigned int)sig->digest->length != m_len) ||
			(memcmp(m,sig->digest->data,m_len) != 0))
			{
			RSAerr(RSA_F_INT_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
			}
		else
			ret=1;
	}
err:
	if (sig != NULL) X509_SIG_free(sig);
	if (s != NULL)
		{
		OPENSSL_cleanse(s,(unsigned int)siglen);
		OPENSSL_free(s);
		}
	return(ret);
	}

int RSA_verify(int dtype, const unsigned char *m, unsigned int m_len,
		const unsigned char *sigbuf, unsigned int siglen,
		RSA *rsa)
	{

	if((rsa->flags & RSA_FLAG_SIGN_VER) && rsa->meth->rsa_verify)
		{
		return rsa->meth->rsa_verify(dtype, m, m_len,
			sigbuf, siglen, rsa);
		}

	return int_rsa_verify(dtype, m, m_len, NULL, NULL, sigbuf, siglen, rsa);
	}