
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

#include <openssl/asn1.h>
#include <openssl/pkcs7.h>
#include <openssl/bio.h>

#if !defined(OPENSSL_SYSNAME_NETWARE) && !defined(OPENSSL_SYSNAME_VXWORKS)
#include <memory.h>
#endif
#include <stdio.h>

/* Streaming encode support for PKCS#7 */

BIO *BIO_new_PKCS7(BIO *out, PKCS7 *p7) 
	{
	return BIO_new_NDEF(out, (ASN1_VALUE *)p7, ASN1_ITEM_rptr(PKCS7));
	}