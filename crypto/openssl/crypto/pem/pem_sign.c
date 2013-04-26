
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
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

void PEM_SignInit(EVP_MD_CTX *ctx, EVP_MD *type)
	{
	EVP_DigestInit_ex(ctx, type, NULL);
	}

void PEM_SignUpdate(EVP_MD_CTX *ctx, unsigned char *data,
	     unsigned int count)
	{
	EVP_DigestUpdate(ctx,data,count);
	}

int PEM_SignFinal(EVP_MD_CTX *ctx, unsigned char *sigret, unsigned int *siglen,
	     EVP_PKEY *pkey)
	{
	unsigned char *m;
	int i,ret=0;
	unsigned int m_len;

	m=(unsigned char *)OPENSSL_malloc(EVP_PKEY_size(pkey)+2);
	if (m == NULL)
		{
		PEMerr(PEM_F_PEM_SIGNFINAL,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	if (EVP_SignFinal(ctx,m,&m_len,pkey) <= 0) goto err;

	i=EVP_EncodeBlock(sigret,m,m_len);
	*siglen=i;
	ret=1;
err:
	/* ctx has been zeroed by EVP_SignFinal() */
	if (m != NULL) OPENSSL_free(m);
	return(ret);
	}