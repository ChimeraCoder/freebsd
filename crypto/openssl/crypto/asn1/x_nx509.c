
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

#include <stddef.h>
#include <openssl/x509.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>

/* Old netscape certificate wrapper format */

ASN1_SEQUENCE(NETSCAPE_X509) = {
	ASN1_SIMPLE(NETSCAPE_X509, header, ASN1_OCTET_STRING),
	ASN1_OPT(NETSCAPE_X509, cert, X509)
} ASN1_SEQUENCE_END(NETSCAPE_X509)

IMPLEMENT_ASN1_FUNCTIONS(NETSCAPE_X509)