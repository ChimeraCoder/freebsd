
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
#include <openssl/conf.h>
#include <openssl/x509v3.h>

static BIT_STRING_BITNAME ns_cert_type_table[] = {
{0, "SSL Client", "client"},
{1, "SSL Server", "server"},
{2, "S/MIME", "email"},
{3, "Object Signing", "objsign"},
{4, "Unused", "reserved"},
{5, "SSL CA", "sslCA"},
{6, "S/MIME CA", "emailCA"},
{7, "Object Signing CA", "objCA"},
{-1, NULL, NULL}
};

static BIT_STRING_BITNAME key_usage_type_table[] = {
{0, "Digital Signature", "digitalSignature"},
{1, "Non Repudiation", "nonRepudiation"},
{2, "Key Encipherment", "keyEncipherment"},
{3, "Data Encipherment", "dataEncipherment"},
{4, "Key Agreement", "keyAgreement"},
{5, "Certificate Sign", "keyCertSign"},
{6, "CRL Sign", "cRLSign"},
{7, "Encipher Only", "encipherOnly"},
{8, "Decipher Only", "decipherOnly"},
{-1, NULL, NULL}
};



const X509V3_EXT_METHOD v3_nscert = EXT_BITSTRING(NID_netscape_cert_type, ns_cert_type_table);
const X509V3_EXT_METHOD v3_key_usage = EXT_BITSTRING(NID_key_usage, key_usage_type_table);

STACK_OF(CONF_VALUE) *i2v_ASN1_BIT_STRING(X509V3_EXT_METHOD *method,
	     ASN1_BIT_STRING *bits, STACK_OF(CONF_VALUE) *ret)
{
	BIT_STRING_BITNAME *bnam;
	for(bnam =method->usr_data; bnam->lname; bnam++) {
		if(ASN1_BIT_STRING_get_bit(bits, bnam->bitnum)) 
			X509V3_add_value(bnam->lname, NULL, &ret);
	}
	return ret;
}
	
ASN1_BIT_STRING *v2i_ASN1_BIT_STRING(X509V3_EXT_METHOD *method,
	     X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *nval)
{
	CONF_VALUE *val;
	ASN1_BIT_STRING *bs;
	int i;
	BIT_STRING_BITNAME *bnam;
	if(!(bs = M_ASN1_BIT_STRING_new())) {
		X509V3err(X509V3_F_V2I_ASN1_BIT_STRING,ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	for(i = 0; i < sk_CONF_VALUE_num(nval); i++) {
		val = sk_CONF_VALUE_value(nval, i);
		for(bnam = method->usr_data; bnam->lname; bnam++) {
			if(!strcmp(bnam->sname, val->name) ||
				!strcmp(bnam->lname, val->name) ) {
				if(!ASN1_BIT_STRING_set_bit(bs, bnam->bitnum, 1)) {
					X509V3err(X509V3_F_V2I_ASN1_BIT_STRING,
						ERR_R_MALLOC_FAILURE);
					M_ASN1_BIT_STRING_free(bs);
					return NULL;
				}
				break;
			}
		}
		if(!bnam->lname) {
			X509V3err(X509V3_F_V2I_ASN1_BIT_STRING,
					X509V3_R_UNKNOWN_BIT_STRING_ARGUMENT);
			X509V3_conf_err(val);
			M_ASN1_BIT_STRING_free(bs);
			return NULL;
		}
	}
	return bs;
}
	