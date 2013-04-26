
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
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>

int X509_certificate_type(X509 *x, EVP_PKEY *pkey)
	{
	EVP_PKEY *pk;
	int ret=0,i;

	if (x == NULL) return(0);

	if (pkey == NULL)
		pk=X509_get_pubkey(x);
	else
		pk=pkey;

	if (pk == NULL) return(0);

	switch (pk->type)
		{
	case EVP_PKEY_RSA:
		ret=EVP_PK_RSA|EVP_PKT_SIGN;
/*		if (!sign only extension) */
			ret|=EVP_PKT_ENC;
	break;
	case EVP_PKEY_DSA:
		ret=EVP_PK_DSA|EVP_PKT_SIGN;
		break;
	case EVP_PKEY_EC:
		ret=EVP_PK_EC|EVP_PKT_SIGN|EVP_PKT_EXCH;
		break;
	case EVP_PKEY_DH:
		ret=EVP_PK_DH|EVP_PKT_EXCH;
		break;	
	case NID_id_GostR3410_94:
	case NID_id_GostR3410_2001:
		ret=EVP_PKT_EXCH|EVP_PKT_SIGN;
		break;
	default:
		break;
		}

	i=OBJ_obj2nid(x->sig_alg->algorithm);
	if (i && OBJ_find_sigid_algs(i, NULL, &i))
		{

		switch (i)
			{
		case NID_rsaEncryption:
		case NID_rsa:
			ret|=EVP_PKS_RSA;
			break;
		case NID_dsa:
		case NID_dsa_2:
			ret|=EVP_PKS_DSA;
			break;
		case NID_X9_62_id_ecPublicKey:
			ret|=EVP_PKS_EC;
			break;
		default:
			break;
			}
		}

	if (EVP_PKEY_size(pk) <= 1024/8)/* /8 because it's 1024 bits we look
					   for, not bytes */
		ret|=EVP_PKT_EXP;
	if(pkey==NULL) EVP_PKEY_free(pk);
	return(ret);
	}