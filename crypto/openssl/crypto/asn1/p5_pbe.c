
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
#include <openssl/rand.h>

/* PKCS#5 password based encryption structure */

ASN1_SEQUENCE(PBEPARAM) = {
	ASN1_SIMPLE(PBEPARAM, salt, ASN1_OCTET_STRING),
	ASN1_SIMPLE(PBEPARAM, iter, ASN1_INTEGER)
} ASN1_SEQUENCE_END(PBEPARAM)

IMPLEMENT_ASN1_FUNCTIONS(PBEPARAM)


/* Set an algorithm identifier for a PKCS#5 PBE algorithm */

int PKCS5_pbe_set0_algor(X509_ALGOR *algor, int alg, int iter,
				const unsigned char *salt, int saltlen)
	{
	PBEPARAM *pbe=NULL;
	ASN1_STRING *pbe_str=NULL;
	unsigned char *sstr;

	pbe = PBEPARAM_new();
	if (!pbe)
		{
		ASN1err(ASN1_F_PKCS5_PBE_SET0_ALGOR,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	if(iter <= 0)
		iter = PKCS5_DEFAULT_ITER;
	if (!ASN1_INTEGER_set(pbe->iter, iter))
		{
		ASN1err(ASN1_F_PKCS5_PBE_SET0_ALGOR,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	if (!saltlen)
		saltlen = PKCS5_SALT_LEN;
	if (!ASN1_STRING_set(pbe->salt, NULL, saltlen))
		{
		ASN1err(ASN1_F_PKCS5_PBE_SET0_ALGOR,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	sstr = ASN1_STRING_data(pbe->salt);
	if (salt)
		memcpy(sstr, salt, saltlen);
	else if (RAND_pseudo_bytes(sstr, saltlen) < 0)
		goto err;

	if(!ASN1_item_pack(pbe, ASN1_ITEM_rptr(PBEPARAM), &pbe_str))
		{
		ASN1err(ASN1_F_PKCS5_PBE_SET0_ALGOR,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	PBEPARAM_free(pbe);
	pbe = NULL;

	if (X509_ALGOR_set0(algor, OBJ_nid2obj(alg), V_ASN1_SEQUENCE, pbe_str))
		return 1;

err:
	if (pbe != NULL)
		PBEPARAM_free(pbe);
	if (pbe_str != NULL)
		ASN1_STRING_free(pbe_str);
	return 0;
	}

/* Return an algorithm identifier for a PKCS#5 PBE algorithm */

X509_ALGOR *PKCS5_pbe_set(int alg, int iter,
				const unsigned char *salt, int saltlen)
	{
	X509_ALGOR *ret;
	ret = X509_ALGOR_new();
	if (!ret)
		{
		ASN1err(ASN1_F_PKCS5_PBE_SET,ERR_R_MALLOC_FAILURE);
		return NULL;
		}

	if (PKCS5_pbe_set0_algor(ret, alg, iter, salt, saltlen)) 
		return ret;

	X509_ALGOR_free(ret);
	return NULL;
	}