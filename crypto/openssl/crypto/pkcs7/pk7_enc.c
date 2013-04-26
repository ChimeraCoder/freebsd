
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
#include <openssl/rand.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/pkcs7.h>

PKCS7_in_bio(PKCS7 *p7,BIO *in);
PKCS7_out_bio(PKCS7 *p7,BIO *out);

PKCS7_add_signer(PKCS7 *p7,X509 *cert,EVP_PKEY *key);
PKCS7_cipher(PKCS7 *p7,EVP_CIPHER *cipher);

PKCS7_Init(PKCS7 *p7);
PKCS7_Update(PKCS7 *p7);
PKCS7_Finish(PKCS7 *p7);