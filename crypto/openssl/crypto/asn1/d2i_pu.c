
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
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/asn1.h>
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#ifndef OPENSSL_NO_DSA
#include <openssl/dsa.h>
#endif
#ifndef OPENSSL_NO_EC
#include <openssl/ec.h>
#endif

EVP_PKEY *d2i_PublicKey(int type, EVP_PKEY **a, const unsigned char **pp,
	     long length)
	{
	EVP_PKEY *ret;

	if ((a == NULL) || (*a == NULL))
		{
		if ((ret=EVP_PKEY_new()) == NULL)
			{
			ASN1err(ASN1_F_D2I_PUBLICKEY,ERR_R_EVP_LIB);
			return(NULL);
			}
		}
	else	ret= *a;

	if (!EVP_PKEY_set_type(ret, type))
		{
		ASN1err(ASN1_F_D2I_PUBLICKEY,ERR_R_EVP_LIB);
		goto err;
		}

	switch (EVP_PKEY_id(ret))
		{
#ifndef OPENSSL_NO_RSA
	case EVP_PKEY_RSA:
		if ((ret->pkey.rsa=d2i_RSAPublicKey(NULL,
			(const unsigned char **)pp,length)) == NULL) /* TMP UGLY CAST */
			{
			ASN1err(ASN1_F_D2I_PUBLICKEY,ERR_R_ASN1_LIB);
			goto err;
			}
		break;
#endif
#ifndef OPENSSL_NO_DSA
	case EVP_PKEY_DSA:
		if (!d2i_DSAPublicKey(&(ret->pkey.dsa),
			(const unsigned char **)pp,length)) /* TMP UGLY CAST */
			{
			ASN1err(ASN1_F_D2I_PUBLICKEY,ERR_R_ASN1_LIB);
			goto err;
			}
		break;
#endif
#ifndef OPENSSL_NO_EC
	case EVP_PKEY_EC:
		if (!o2i_ECPublicKey(&(ret->pkey.ec),
				     (const unsigned char **)pp, length))
			{
			ASN1err(ASN1_F_D2I_PUBLICKEY, ERR_R_ASN1_LIB);
			goto err;
			}
	break;
#endif
	default:
		ASN1err(ASN1_F_D2I_PUBLICKEY,ASN1_R_UNKNOWN_PUBLIC_KEY_TYPE);
		goto err;
		/* break; */
		}
	if (a != NULL) (*a)=ret;
	return(ret);
err:
	if ((ret != NULL) && ((a == NULL) || (*a != ret))) EVP_PKEY_free(ret);
	return(NULL);
	}