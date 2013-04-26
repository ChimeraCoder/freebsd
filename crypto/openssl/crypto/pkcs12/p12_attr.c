
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
#include <openssl/pkcs12.h>

/* Add a local keyid to a safebag */

int PKCS12_add_localkeyid(PKCS12_SAFEBAG *bag, unsigned char *name,
	     int namelen)
{
	if (X509at_add1_attr_by_NID(&bag->attrib, NID_localKeyID,
				V_ASN1_OCTET_STRING, name, namelen))
		return 1;
	else 
		return 0;
}

/* Add key usage to PKCS#8 structure */

int PKCS8_add_keyusage(PKCS8_PRIV_KEY_INFO *p8, int usage)
{
	unsigned char us_val;
	us_val = (unsigned char) usage;
	if (X509at_add1_attr_by_NID(&p8->attributes, NID_key_usage,
				V_ASN1_BIT_STRING, &us_val, 1))
		return 1;
	else
		return 0;
}

/* Add a friendlyname to a safebag */

int PKCS12_add_friendlyname_asc(PKCS12_SAFEBAG *bag, const char *name,
				 int namelen)
{
	if (X509at_add1_attr_by_NID(&bag->attrib, NID_friendlyName,
				MBSTRING_ASC, (unsigned char *)name, namelen))
		return 1;
	else
		return 0;
}


int PKCS12_add_friendlyname_uni(PKCS12_SAFEBAG *bag,
				 const unsigned char *name, int namelen)
{
	if (X509at_add1_attr_by_NID(&bag->attrib, NID_friendlyName,
				MBSTRING_BMP, name, namelen))
		return 1;
	else
		return 0;
}

int PKCS12_add_CSPName_asc(PKCS12_SAFEBAG *bag, const char *name,
				 int namelen)
{
	if (X509at_add1_attr_by_NID(&bag->attrib, NID_ms_csp_name,
				MBSTRING_ASC, (unsigned char *)name, namelen))
		return 1;
	else
		return 0;
}

ASN1_TYPE *PKCS12_get_attr_gen(STACK_OF(X509_ATTRIBUTE) *attrs, int attr_nid)
{
	X509_ATTRIBUTE *attrib;
	int i;
	if (!attrs) return NULL;
	for (i = 0; i < sk_X509_ATTRIBUTE_num (attrs); i++) {
		attrib = sk_X509_ATTRIBUTE_value (attrs, i);
		if (OBJ_obj2nid (attrib->object) == attr_nid) {
			if (sk_ASN1_TYPE_num (attrib->value.set))
			    return sk_ASN1_TYPE_value(attrib->value.set, 0);
			else return NULL;
		}
	}
	return NULL;
}

char *PKCS12_get_friendlyname(PKCS12_SAFEBAG *bag)
{
	ASN1_TYPE *atype;
	if (!(atype = PKCS12_get_attr(bag, NID_friendlyName))) return NULL;
	if (atype->type != V_ASN1_BMPSTRING) return NULL;
	return OPENSSL_uni2asc(atype->value.bmpstring->data,
				 atype->value.bmpstring->length);
}