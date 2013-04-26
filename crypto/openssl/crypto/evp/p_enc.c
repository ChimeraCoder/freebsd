
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

int EVP_PKEY_encrypt_old(unsigned char *ek, const unsigned char *key, int key_len,
	     EVP_PKEY *pubk)
	{
	int ret=0;
	
#ifndef OPENSSL_NO_RSA
	if (pubk->type != EVP_PKEY_RSA)
		{
#endif
		EVPerr(EVP_F_EVP_PKEY_ENCRYPT_OLD,EVP_R_PUBLIC_KEY_NOT_RSA);
#ifndef OPENSSL_NO_RSA
		goto err;
		}
	ret=RSA_public_encrypt(key_len,key,ek,pubk->pkey.rsa,RSA_PKCS1_PADDING);
err:
#endif
	return(ret);
	}