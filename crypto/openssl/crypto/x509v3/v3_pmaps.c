
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
#include <openssl/conf.h>
#include <openssl/x509v3.h>

static void *v2i_POLICY_MAPPINGS(const X509V3_EXT_METHOD *method,
				 X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *nval);
static STACK_OF(CONF_VALUE) *
i2v_POLICY_MAPPINGS(const X509V3_EXT_METHOD *method, void *pmps,
		    STACK_OF(CONF_VALUE) *extlist);

const X509V3_EXT_METHOD v3_policy_mappings = {
	NID_policy_mappings, 0,
	ASN1_ITEM_ref(POLICY_MAPPINGS),
	0,0,0,0,
	0,0,
	i2v_POLICY_MAPPINGS,
	v2i_POLICY_MAPPINGS,
	0,0,
	NULL
};

ASN1_SEQUENCE(POLICY_MAPPING) = {
	ASN1_SIMPLE(POLICY_MAPPING, issuerDomainPolicy, ASN1_OBJECT),
	ASN1_SIMPLE(POLICY_MAPPING, subjectDomainPolicy, ASN1_OBJECT)
} ASN1_SEQUENCE_END(POLICY_MAPPING)

ASN1_ITEM_TEMPLATE(POLICY_MAPPINGS) = 
	ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_SEQUENCE_OF, 0, POLICY_MAPPINGS,
								POLICY_MAPPING)
ASN1_ITEM_TEMPLATE_END(POLICY_MAPPINGS)

IMPLEMENT_ASN1_ALLOC_FUNCTIONS(POLICY_MAPPING)


static STACK_OF(CONF_VALUE) *
i2v_POLICY_MAPPINGS(const X509V3_EXT_METHOD *method, void *a,
		    STACK_OF(CONF_VALUE) *ext_list)
{
	POLICY_MAPPINGS *pmaps = a;
	POLICY_MAPPING *pmap;
	int i;
	char obj_tmp1[80];
	char obj_tmp2[80];
	for(i = 0; i < sk_POLICY_MAPPING_num(pmaps); i++) {
		pmap = sk_POLICY_MAPPING_value(pmaps, i);
		i2t_ASN1_OBJECT(obj_tmp1, 80, pmap->issuerDomainPolicy);
		i2t_ASN1_OBJECT(obj_tmp2, 80, pmap->subjectDomainPolicy);
		X509V3_add_value(obj_tmp1, obj_tmp2, &ext_list);
	}
	return ext_list;
}

static void *v2i_POLICY_MAPPINGS(const X509V3_EXT_METHOD *method,
				 X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *nval)
{
	POLICY_MAPPINGS *pmaps;
	POLICY_MAPPING *pmap;
	ASN1_OBJECT *obj1, *obj2;
	CONF_VALUE *val;
	int i;

	if(!(pmaps = sk_POLICY_MAPPING_new_null())) {
		X509V3err(X509V3_F_V2I_POLICY_MAPPINGS,ERR_R_MALLOC_FAILURE);
		return NULL;
	}

	for(i = 0; i < sk_CONF_VALUE_num(nval); i++) {
		val = sk_CONF_VALUE_value(nval, i);
		if(!val->value || !val->name) {
			sk_POLICY_MAPPING_pop_free(pmaps, POLICY_MAPPING_free);
			X509V3err(X509V3_F_V2I_POLICY_MAPPINGS,X509V3_R_INVALID_OBJECT_IDENTIFIER);
			X509V3_conf_err(val);
			return NULL;
		}
		obj1 = OBJ_txt2obj(val->name, 0);
		obj2 = OBJ_txt2obj(val->value, 0);
		if(!obj1 || !obj2) {
			sk_POLICY_MAPPING_pop_free(pmaps, POLICY_MAPPING_free);
			X509V3err(X509V3_F_V2I_POLICY_MAPPINGS,X509V3_R_INVALID_OBJECT_IDENTIFIER);
			X509V3_conf_err(val);
			return NULL;
		}
		pmap = POLICY_MAPPING_new();
		if (!pmap) {
			sk_POLICY_MAPPING_pop_free(pmaps, POLICY_MAPPING_free);
			X509V3err(X509V3_F_V2I_POLICY_MAPPINGS,ERR_R_MALLOC_FAILURE);
			return NULL;
		}
		pmap->issuerDomainPolicy = obj1;
		pmap->subjectDomainPolicy = obj2;
		sk_POLICY_MAPPING_push(pmaps, pmap);
	}
	return pmaps;
}