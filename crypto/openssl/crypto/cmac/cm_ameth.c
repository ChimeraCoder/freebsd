
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
#include <openssl/cmac.h>
#include "asn1_locl.h"

/* CMAC "ASN1" method. This is just here to indicate the
 * maximum CMAC output length and to free up a CMAC
 * key.
 */

static int cmac_size(const EVP_PKEY *pkey)
	{
	return EVP_MAX_BLOCK_LENGTH;
	}

static void cmac_key_free(EVP_PKEY *pkey)
	{
	CMAC_CTX *cmctx = (CMAC_CTX *)pkey->pkey.ptr;
	if (cmctx)
		CMAC_CTX_free(cmctx);
	}

const EVP_PKEY_ASN1_METHOD cmac_asn1_meth = 
	{
	EVP_PKEY_CMAC,
	EVP_PKEY_CMAC,
	0,

	"CMAC",
	"OpenSSL CMAC method",

	0,0,0,0,

	0,0,0,

	cmac_size,
	0,
	0,0,0,0,0,0,0,

	cmac_key_free,
	0,
	0,0
	};