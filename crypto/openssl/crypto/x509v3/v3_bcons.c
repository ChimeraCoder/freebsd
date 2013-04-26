
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
#include <openssl/asn1t.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>

static STACK_OF(CONF_VALUE) *i2v_BASIC_CONSTRAINTS(X509V3_EXT_METHOD *method, BASIC_CONSTRAINTS *bcons, STACK_OF(CONF_VALUE) *extlist);
static BASIC_CONSTRAINTS *v2i_BASIC_CONSTRAINTS(X509V3_EXT_METHOD *method, X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *values);

const X509V3_EXT_METHOD v3_bcons = {
NID_basic_constraints, 0,
ASN1_ITEM_ref(BASIC_CONSTRAINTS),
0,0,0,0,
0,0,
(X509V3_EXT_I2V)i2v_BASIC_CONSTRAINTS,
(X509V3_EXT_V2I)v2i_BASIC_CONSTRAINTS,
NULL,NULL,
NULL
};

ASN1_SEQUENCE(BASIC_CONSTRAINTS) = {
	ASN1_OPT(BASIC_CONSTRAINTS, ca, ASN1_FBOOLEAN),
	ASN1_OPT(BASIC_CONSTRAINTS, pathlen, ASN1_INTEGER)
} ASN1_SEQUENCE_END(BASIC_CONSTRAINTS)

IMPLEMENT_ASN1_FUNCTIONS(BASIC_CONSTRAINTS)


static STACK_OF(CONF_VALUE) *i2v_BASIC_CONSTRAINTS(X509V3_EXT_METHOD *method,
	     BASIC_CONSTRAINTS *bcons, STACK_OF(CONF_VALUE) *extlist)
{
	X509V3_add_value_bool("CA", bcons->ca, &extlist);
	X509V3_add_value_int("pathlen", bcons->pathlen, &extlist);
	return extlist;
}

static BASIC_CONSTRAINTS *v2i_BASIC_CONSTRAINTS(X509V3_EXT_METHOD *method,
	     X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *values)
{
	BASIC_CONSTRAINTS *bcons=NULL;
	CONF_VALUE *val;
	int i;
	if(!(bcons = BASIC_CONSTRAINTS_new())) {
		X509V3err(X509V3_F_V2I_BASIC_CONSTRAINTS, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	for(i = 0; i < sk_CONF_VALUE_num(values); i++) {
		val = sk_CONF_VALUE_value(values, i);
		if(!strcmp(val->name, "CA")) {
			if(!X509V3_get_value_bool(val, &bcons->ca)) goto err;
		} else if(!strcmp(val->name, "pathlen")) {
			if(!X509V3_get_value_int(val, &bcons->pathlen)) goto err;
		} else {
			X509V3err(X509V3_F_V2I_BASIC_CONSTRAINTS, X509V3_R_INVALID_NAME);
			X509V3_conf_err(val);
			goto err;
		}
	}
	return bcons;
	err:
	BASIC_CONSTRAINTS_free(bcons);
	return NULL;
}