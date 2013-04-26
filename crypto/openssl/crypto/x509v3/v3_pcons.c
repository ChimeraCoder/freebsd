
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

static STACK_OF(CONF_VALUE) *
i2v_POLICY_CONSTRAINTS(const X509V3_EXT_METHOD *method, void *bcons,
		       STACK_OF(CONF_VALUE) *extlist);
static void *v2i_POLICY_CONSTRAINTS(const X509V3_EXT_METHOD *method,
				    X509V3_CTX *ctx,
				    STACK_OF(CONF_VALUE) *values);

const X509V3_EXT_METHOD v3_policy_constraints = {
NID_policy_constraints, 0,
ASN1_ITEM_ref(POLICY_CONSTRAINTS),
0,0,0,0,
0,0,
i2v_POLICY_CONSTRAINTS,
v2i_POLICY_CONSTRAINTS,
NULL,NULL,
NULL
};

ASN1_SEQUENCE(POLICY_CONSTRAINTS) = {
	ASN1_IMP_OPT(POLICY_CONSTRAINTS, requireExplicitPolicy, ASN1_INTEGER,0),
	ASN1_IMP_OPT(POLICY_CONSTRAINTS, inhibitPolicyMapping, ASN1_INTEGER,1)
} ASN1_SEQUENCE_END(POLICY_CONSTRAINTS)

IMPLEMENT_ASN1_ALLOC_FUNCTIONS(POLICY_CONSTRAINTS)


static STACK_OF(CONF_VALUE) *
i2v_POLICY_CONSTRAINTS(const X509V3_EXT_METHOD *method, void *a,
		       STACK_OF(CONF_VALUE) *extlist)
{
	POLICY_CONSTRAINTS *pcons = a;
	X509V3_add_value_int("Require Explicit Policy",
			pcons->requireExplicitPolicy, &extlist);
	X509V3_add_value_int("Inhibit Policy Mapping",
			pcons->inhibitPolicyMapping, &extlist);
	return extlist;
}

static void *v2i_POLICY_CONSTRAINTS(const X509V3_EXT_METHOD *method,
				    X509V3_CTX *ctx,
				    STACK_OF(CONF_VALUE) *values)
{
	POLICY_CONSTRAINTS *pcons=NULL;
	CONF_VALUE *val;
	int i;
	if(!(pcons = POLICY_CONSTRAINTS_new())) {
		X509V3err(X509V3_F_V2I_POLICY_CONSTRAINTS, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	for(i = 0; i < sk_CONF_VALUE_num(values); i++) {
		val = sk_CONF_VALUE_value(values, i);
		if(!strcmp(val->name, "requireExplicitPolicy")) {
			if(!X509V3_get_value_int(val,
				&pcons->requireExplicitPolicy)) goto err;
		} else if(!strcmp(val->name, "inhibitPolicyMapping")) {
			if(!X509V3_get_value_int(val,
				&pcons->inhibitPolicyMapping)) goto err;
		} else {
			X509V3err(X509V3_F_V2I_POLICY_CONSTRAINTS, X509V3_R_INVALID_NAME);
			X509V3_conf_err(val);
			goto err;
		}
	}
	if (!pcons->inhibitPolicyMapping && !pcons->requireExplicitPolicy) {
		X509V3err(X509V3_F_V2I_POLICY_CONSTRAINTS, X509V3_R_ILLEGAL_EMPTY_EXTENSION);
		goto err;
	}

	return pcons;
	err:
	POLICY_CONSTRAINTS_free(pcons);
	return NULL;
}