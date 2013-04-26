
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
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>

int EVP_SealInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *type, unsigned char **ek,
	     int *ekl, unsigned char *iv, EVP_PKEY **pubk, int npubk)
	{
	unsigned char key[EVP_MAX_KEY_LENGTH];
	int i;
	
	if(type) {
		EVP_CIPHER_CTX_init(ctx);
		if(!EVP_EncryptInit_ex(ctx,type,NULL,NULL,NULL)) return 0;
	}
	if ((npubk <= 0) || !pubk)
		return 1;
	if (EVP_CIPHER_CTX_rand_key(ctx, key) <= 0)
		return 0;
	if (EVP_CIPHER_CTX_iv_length(ctx))
		RAND_pseudo_bytes(iv,EVP_CIPHER_CTX_iv_length(ctx));

	if(!EVP_EncryptInit_ex(ctx,NULL,NULL,key,iv)) return 0;

	for (i=0; i<npubk; i++)
		{
		ekl[i]=EVP_PKEY_encrypt_old(ek[i],key,EVP_CIPHER_CTX_key_length(ctx),
			pubk[i]);
		if (ekl[i] <= 0) return(-1);
		}
	return(npubk);
	}

/* MACRO
void EVP_SealUpdate(ctx,out,outl,in,inl)
EVP_CIPHER_CTX *ctx;
unsigned char *out;
int *outl;
unsigned char *in;
int inl;
	{
	EVP_EncryptUpdate(ctx,out,outl,in,inl);
	}
*/

int EVP_SealFinal(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl)
	{
	int i;
	i = EVP_EncryptFinal_ex(ctx,out,outl);
	if (i) 
		i = EVP_EncryptInit_ex(ctx,NULL,NULL,NULL,NULL);
	return i;
	}