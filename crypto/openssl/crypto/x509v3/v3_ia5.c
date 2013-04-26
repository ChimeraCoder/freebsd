
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
#include <openssl/asn1.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>

static char *i2s_ASN1_IA5STRING(X509V3_EXT_METHOD *method, ASN1_IA5STRING *ia5);
static ASN1_IA5STRING *s2i_ASN1_IA5STRING(X509V3_EXT_METHOD *method, X509V3_CTX *ctx, char *str);
const X509V3_EXT_METHOD v3_ns_ia5_list[] = { 
EXT_IA5STRING(NID_netscape_base_url),
EXT_IA5STRING(NID_netscape_revocation_url),
EXT_IA5STRING(NID_netscape_ca_revocation_url),
EXT_IA5STRING(NID_netscape_renewal_url),
EXT_IA5STRING(NID_netscape_ca_policy_url),
EXT_IA5STRING(NID_netscape_ssl_server_name),
EXT_IA5STRING(NID_netscape_comment),
EXT_END
};


static char *i2s_ASN1_IA5STRING(X509V3_EXT_METHOD *method,
	     ASN1_IA5STRING *ia5)
{
	char *tmp;
	if(!ia5 || !ia5->length) return NULL;
	if(!(tmp = OPENSSL_malloc(ia5->length + 1))) {
		X509V3err(X509V3_F_I2S_ASN1_IA5STRING,ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	memcpy(tmp, ia5->data, ia5->length);
	tmp[ia5->length] = 0;
	return tmp;
}

static ASN1_IA5STRING *s2i_ASN1_IA5STRING(X509V3_EXT_METHOD *method,
	     X509V3_CTX *ctx, char *str)
{
	ASN1_IA5STRING *ia5;
	if(!str) {
		X509V3err(X509V3_F_S2I_ASN1_IA5STRING,X509V3_R_INVALID_NULL_ARGUMENT);
		return NULL;
	}
	if(!(ia5 = M_ASN1_IA5STRING_new())) goto err;
	if(!ASN1_STRING_set((ASN1_STRING *)ia5, (unsigned char*)str,
			    strlen(str))) {
		M_ASN1_IA5STRING_free(ia5);
		goto err;
	}
#ifdef CHARSET_EBCDIC
        ebcdic2ascii(ia5->data, ia5->data, ia5->length);
#endif /*CHARSET_EBCDIC*/
	return ia5;
	err:
	X509V3err(X509V3_F_S2I_ASN1_IA5STRING,ERR_R_MALLOC_FAILURE);
	return NULL;
}