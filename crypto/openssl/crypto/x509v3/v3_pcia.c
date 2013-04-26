
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
#include <openssl/asn1t.h>
#include <openssl/x509v3.h>

ASN1_SEQUENCE(PROXY_POLICY) =
	{
	ASN1_SIMPLE(PROXY_POLICY,policyLanguage,ASN1_OBJECT),
	ASN1_OPT(PROXY_POLICY,policy,ASN1_OCTET_STRING)
} ASN1_SEQUENCE_END(PROXY_POLICY)

IMPLEMENT_ASN1_FUNCTIONS(PROXY_POLICY)

ASN1_SEQUENCE(PROXY_CERT_INFO_EXTENSION) =
	{
	ASN1_OPT(PROXY_CERT_INFO_EXTENSION,pcPathLengthConstraint,ASN1_INTEGER),
	ASN1_SIMPLE(PROXY_CERT_INFO_EXTENSION,proxyPolicy,PROXY_POLICY)
} ASN1_SEQUENCE_END(PROXY_CERT_INFO_EXTENSION)

IMPLEMENT_ASN1_FUNCTIONS(PROXY_CERT_INFO_EXTENSION)