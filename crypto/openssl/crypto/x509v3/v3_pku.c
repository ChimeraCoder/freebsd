
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
#include <openssl/x509v3.h>

static int i2r_PKEY_USAGE_PERIOD(X509V3_EXT_METHOD *method, PKEY_USAGE_PERIOD *usage, BIO *out, int indent);
/*
static PKEY_USAGE_PERIOD *v2i_PKEY_USAGE_PERIOD(X509V3_EXT_METHOD *method, X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *values);
*/
const X509V3_EXT_METHOD v3_pkey_usage_period = {
NID_private_key_usage_period, 0, ASN1_ITEM_ref(PKEY_USAGE_PERIOD),
0,0,0,0,
0,0,0,0,
(X509V3_EXT_I2R)i2r_PKEY_USAGE_PERIOD, NULL,
NULL
};

ASN1_SEQUENCE(PKEY_USAGE_PERIOD) = {
	ASN1_IMP_OPT(PKEY_USAGE_PERIOD, notBefore, ASN1_GENERALIZEDTIME, 0),
	ASN1_IMP_OPT(PKEY_USAGE_PERIOD, notAfter, ASN1_GENERALIZEDTIME, 1)
} ASN1_SEQUENCE_END(PKEY_USAGE_PERIOD)

IMPLEMENT_ASN1_FUNCTIONS(PKEY_USAGE_PERIOD)

static int i2r_PKEY_USAGE_PERIOD(X509V3_EXT_METHOD *method,
	     PKEY_USAGE_PERIOD *usage, BIO *out, int indent)
{
	BIO_printf(out, "%*s", indent, "");
	if(usage->notBefore) {
		BIO_write(out, "Not Before: ", 12);
		ASN1_GENERALIZEDTIME_print(out, usage->notBefore);
		if(usage->notAfter) BIO_write(out, ", ", 2);
	}
	if(usage->notAfter) {
		BIO_write(out, "Not After: ", 11);
		ASN1_GENERALIZEDTIME_print(out, usage->notAfter);
	}
	return 1;
}

/*
static PKEY_USAGE_PERIOD *v2i_PKEY_USAGE_PERIOD(method, ctx, values)
X509V3_EXT_METHOD *method;
X509V3_CTX *ctx;
STACK_OF(CONF_VALUE) *values;
{
return NULL;
}
*/