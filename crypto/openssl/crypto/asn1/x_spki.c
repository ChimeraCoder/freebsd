
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

 /* This module was send to me my Pat Richards <patr@x509.com> who
  * wrote it.  It is under my Copyright with his permission
  */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/x509.h>
#include <openssl/asn1t.h>

ASN1_SEQUENCE(NETSCAPE_SPKAC) = {
	ASN1_SIMPLE(NETSCAPE_SPKAC, pubkey, X509_PUBKEY),
	ASN1_SIMPLE(NETSCAPE_SPKAC, challenge, ASN1_IA5STRING)
} ASN1_SEQUENCE_END(NETSCAPE_SPKAC)

IMPLEMENT_ASN1_FUNCTIONS(NETSCAPE_SPKAC)

ASN1_SEQUENCE(NETSCAPE_SPKI) = {
	ASN1_SIMPLE(NETSCAPE_SPKI, spkac, NETSCAPE_SPKAC),
	ASN1_SIMPLE(NETSCAPE_SPKI, sig_algor, X509_ALGOR),
	ASN1_SIMPLE(NETSCAPE_SPKI, signature, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END(NETSCAPE_SPKI)

IMPLEMENT_ASN1_FUNCTIONS(NETSCAPE_SPKI)