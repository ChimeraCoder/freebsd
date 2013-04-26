
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
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#ifndef OPENSSL_NO_DSA
#include <openssl/dsa.h>
#endif
#ifndef OPENSSL_NO_EC
#include <openssl/ec.h>
#endif

int i2d_PublicKey(EVP_PKEY *a, unsigned char **pp)
	{
	switch (a->type)
		{
#ifndef OPENSSL_NO_RSA
	case EVP_PKEY_RSA:
		return(i2d_RSAPublicKey(a->pkey.rsa,pp));
#endif
#ifndef OPENSSL_NO_DSA
	case EVP_PKEY_DSA:
		return(i2d_DSAPublicKey(a->pkey.dsa,pp));
#endif
#ifndef OPENSSL_NO_EC
	case EVP_PKEY_EC:
		return(i2o_ECPublicKey(a->pkey.ec, pp));
#endif
	default:
		ASN1err(ASN1_F_I2D_PUBLICKEY,ASN1_R_UNSUPPORTED_PUBLIC_KEY_TYPE);
		return(-1);
		}
	}