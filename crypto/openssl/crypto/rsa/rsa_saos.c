
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

int RSA_sign_ASN1_OCTET_STRING(int type,
	const unsigned char *m, unsigned int m_len,
	unsigned char *sigret, unsigned int *siglen, RSA *rsa)
	{
	ASN1_OCTET_STRING sig;
	int i,j,ret=1;
	unsigned char *p,*s;

	sig.type=V_ASN1_OCTET_STRING;
	sig.length=m_len;
	sig.data=(unsigned char *)m;

	i=i2d_ASN1_OCTET_STRING(&sig,NULL);
	j=RSA_size(rsa);
	if (i > (j-RSA_PKCS1_PADDING_SIZE))
		{
		RSAerr(RSA_F_RSA_SIGN_ASN1_OCTET_STRING,RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY);
		return(0);
		}
	s=(unsigned char *)OPENSSL_malloc((unsigned int)j+1);
	if (s == NULL)
		{
		RSAerr(RSA_F_RSA_SIGN_ASN1_OCTET_STRING,ERR_R_MALLOC_FAILURE);
		return(0);
		}
	p=s;
	i2d_ASN1_OCTET_STRING(&sig,&p);
	i=RSA_private_encrypt(i,s,sigret,rsa,RSA_PKCS1_PADDING);
	if (i <= 0)
		ret=0;
	else
		*siglen=i;

	OPENSSL_cleanse(s,(unsigned int)j+1);
	OPENSSL_free(s);
	return(ret);
	}

int RSA_verify_ASN1_OCTET_STRING(int dtype,
	const unsigned char *m,
	unsigned int m_len, unsigned char *sigbuf, unsigned int siglen,
	RSA *rsa)
	{
	int i,ret=0;
	unsigned char *s;
	const unsigned char *p;
	ASN1_OCTET_STRING *sig=NULL;

	if (siglen != (unsigned int)RSA_size(rsa))
		{
		RSAerr(RSA_F_RSA_VERIFY_ASN1_OCTET_STRING,RSA_R_WRONG_SIGNATURE_LENGTH);
		return(0);
		}

	s=(unsigned char *)OPENSSL_malloc((unsigned int)siglen);
	if (s == NULL)
		{
		RSAerr(RSA_F_RSA_VERIFY_ASN1_OCTET_STRING,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	i=RSA_public_decrypt((int)siglen,sigbuf,s,rsa,RSA_PKCS1_PADDING);

	if (i <= 0) goto err;

	p=s;
	sig=d2i_ASN1_OCTET_STRING(NULL,&p,(long)i);
	if (sig == NULL) goto err;

	if (	((unsigned int)sig->length != m_len) ||
		(memcmp(m,sig->data,m_len) != 0))
		{
		RSAerr(RSA_F_RSA_VERIFY_ASN1_OCTET_STRING,RSA_R_BAD_SIGNATURE);
		}
	else
		ret=1;
err:
	if (sig != NULL) M_ASN1_OCTET_STRING_free(sig);
	if (s != NULL)
		{
		OPENSSL_cleanse(s,(unsigned int)siglen);
		OPENSSL_free(s);
		}
	return(ret);
	}