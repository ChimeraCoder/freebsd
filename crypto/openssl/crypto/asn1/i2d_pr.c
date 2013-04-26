
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
#include <openssl/x509.h>
#include "asn1_locl.h"

int i2d_PrivateKey(EVP_PKEY *a, unsigned char **pp)
	{
	if (a->ameth && a->ameth->old_priv_encode)
		{
		return a->ameth->old_priv_encode(a, pp);
		}
	if (a->ameth && a->ameth->priv_encode) {
		PKCS8_PRIV_KEY_INFO *p8 = EVP_PKEY2PKCS8(a);
		int ret = i2d_PKCS8_PRIV_KEY_INFO(p8,pp);
		PKCS8_PRIV_KEY_INFO_free(p8);
		return ret;
	}	
	ASN1err(ASN1_F_I2D_PRIVATEKEY,ASN1_R_UNSUPPORTED_PUBLIC_KEY_TYPE);
	return(-1);
	}