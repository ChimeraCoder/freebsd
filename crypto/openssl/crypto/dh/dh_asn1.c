
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
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/objects.h>
#include <openssl/asn1t.h>

/* Override the default free and new methods */
static int dh_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
						void *exarg)
{
	if(operation == ASN1_OP_NEW_PRE) {
		*pval = (ASN1_VALUE *)DH_new();
		if(*pval) return 2;
		return 0;
	} else if(operation == ASN1_OP_FREE_PRE) {
		DH_free((DH *)*pval);
		*pval = NULL;
		return 2;
	}
	return 1;
}

ASN1_SEQUENCE_cb(DHparams, dh_cb) = {
	ASN1_SIMPLE(DH, p, BIGNUM),
	ASN1_SIMPLE(DH, g, BIGNUM),
	ASN1_OPT(DH, length, ZLONG),
} ASN1_SEQUENCE_END_cb(DH, DHparams)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(DH, DHparams, DHparams)

DH *DHparams_dup(DH *dh)
	{
	return ASN1_item_dup(ASN1_ITEM_rptr(DHparams), dh);
	}