
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
#include <openssl/asn1t.h>
#include <openssl/x509.h>

ASN1_SEQUENCE(X509_VAL) = {
	ASN1_SIMPLE(X509_VAL, notBefore, ASN1_TIME),
	ASN1_SIMPLE(X509_VAL, notAfter, ASN1_TIME)
} ASN1_SEQUENCE_END(X509_VAL)

IMPLEMENT_ASN1_FUNCTIONS(X509_VAL)