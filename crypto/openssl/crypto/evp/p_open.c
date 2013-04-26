
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

#ifndef OPENSSL_NO_RSA

#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>

int EVP_OpenInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *type,
	const unsigned char *ek, int ekl, const unsigned char *iv,
	EVP_PKEY *priv)
	{
	unsigned char *key=NULL;
	int i,size=0,ret=0;

	if(type) {	
		EVP_CIPHER_CTX_init(ctx);
		if(!EVP_DecryptInit_ex(ctx,type,NULL, NULL,NULL)) return 0;
	}

	if(!priv) return 1;

	if (priv->type != EVP_PKEY_RSA)
		{
		EVPerr(EVP_F_EVP_OPENINIT,EVP_R_PUBLIC_KEY_NOT_RSA);
		goto err;
                }

	size=RSA_size(priv->pkey.rsa);
	key=(unsigned char *)OPENSSL_malloc(size+2);
	if (key == NULL)
		{
		/* ERROR */
		EVPerr(EVP_F_EVP_OPENINIT,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	i=EVP_PKEY_decrypt_old(key,ek,ekl,priv);
	if ((i <= 0) || !EVP_CIPHER_CTX_set_key_length(ctx, i))
		{
		/* ERROR */
		goto err;
		}
	if(!EVP_DecryptInit_ex(ctx,NULL,NULL,key,iv)) goto err;

	ret=1;
err:
	if (key != NULL) OPENSSL_cleanse(key,size);
	OPENSSL_free(key);
	return(ret);
	}

int EVP_OpenFinal(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl)
	{
	int i;

	i=EVP_DecryptFinal_ex(ctx,out,outl);
	if (i)
		i = EVP_DecryptInit_ex(ctx,NULL,NULL,NULL,NULL);
	return(i);
	}
#else /* !OPENSSL_NO_RSA */

# ifdef PEDANTIC
static void *dummy=&dummy;
# endif

#endif