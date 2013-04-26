
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
#include <openssl/rand.h>

/* This is a dummy RSA implementation that just returns errors when called.
 * It is designed to allow some RSA functions to work while stopping those
 * covered by the RSA patent. That is RSA, encryption, decryption, signing
 * and verify is not allowed but RSA key generation, key checking and other
 * operations (like storing RSA keys) are permitted.
 */

static int RSA_null_public_encrypt(int flen, const unsigned char *from,
		unsigned char *to, RSA *rsa,int padding);
static int RSA_null_private_encrypt(int flen, const unsigned char *from,
		unsigned char *to, RSA *rsa,int padding);
static int RSA_null_public_decrypt(int flen, const unsigned char *from,
		unsigned char *to, RSA *rsa,int padding);
static int RSA_null_private_decrypt(int flen, const unsigned char *from,
		unsigned char *to, RSA *rsa,int padding);
#if 0 /* not currently used */
static int RSA_null_mod_exp(const BIGNUM *r0, const BIGNUM *i, RSA *rsa);
#endif
static int RSA_null_init(RSA *rsa);
static int RSA_null_finish(RSA *rsa);
static RSA_METHOD rsa_null_meth={
	"Null RSA",
	RSA_null_public_encrypt,
	RSA_null_public_decrypt,
	RSA_null_private_encrypt,
	RSA_null_private_decrypt,
	NULL,
	NULL,
	RSA_null_init,
	RSA_null_finish,
	0,
	NULL,
	NULL,
	NULL,
	NULL
	};

const RSA_METHOD *RSA_null_method(void)
	{
	return(&rsa_null_meth);
	}

static int RSA_null_public_encrypt(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding)
	{
	RSAerr(RSA_F_RSA_NULL_PUBLIC_ENCRYPT, RSA_R_RSA_OPERATIONS_NOT_SUPPORTED);
	return -1;
	}

static int RSA_null_private_encrypt(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding)
	{
	RSAerr(RSA_F_RSA_NULL_PRIVATE_ENCRYPT, RSA_R_RSA_OPERATIONS_NOT_SUPPORTED);
	return -1;
	}

static int RSA_null_private_decrypt(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding)
	{
	RSAerr(RSA_F_RSA_NULL_PRIVATE_DECRYPT, RSA_R_RSA_OPERATIONS_NOT_SUPPORTED);
	return -1;
	}

static int RSA_null_public_decrypt(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding)
	{
	RSAerr(RSA_F_RSA_NULL_PUBLIC_DECRYPT, RSA_R_RSA_OPERATIONS_NOT_SUPPORTED);
	return -1;
	}

#if 0 /* not currently used */
static int RSA_null_mod_exp(BIGNUM *r0, BIGNUM *I, RSA *rsa)
	{
	...err(RSA_F_RSA_NULL_MOD_EXP, RSA_R_RSA_OPERATIONS_NOT_SUPPORTED);
	return -1;
	}
#endif

static int RSA_null_init(RSA *rsa)
	{
	return(1);
	}

static int RSA_null_finish(RSA *rsa)
	{
	return(1);
	}