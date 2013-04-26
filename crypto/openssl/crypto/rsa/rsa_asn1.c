
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
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/asn1t.h>

/* Override the default free and new methods */
static int rsa_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
								void *exarg)
{
	if(operation == ASN1_OP_NEW_PRE) {
		*pval = (ASN1_VALUE *)RSA_new();
		if(*pval) return 2;
		return 0;
	} else if(operation == ASN1_OP_FREE_PRE) {
		RSA_free((RSA *)*pval);
		*pval = NULL;
		return 2;
	}
	return 1;
}

ASN1_SEQUENCE_cb(RSAPrivateKey, rsa_cb) = {
	ASN1_SIMPLE(RSA, version, LONG),
	ASN1_SIMPLE(RSA, n, BIGNUM),
	ASN1_SIMPLE(RSA, e, BIGNUM),
	ASN1_SIMPLE(RSA, d, BIGNUM),
	ASN1_SIMPLE(RSA, p, BIGNUM),
	ASN1_SIMPLE(RSA, q, BIGNUM),
	ASN1_SIMPLE(RSA, dmp1, BIGNUM),
	ASN1_SIMPLE(RSA, dmq1, BIGNUM),
	ASN1_SIMPLE(RSA, iqmp, BIGNUM)
} ASN1_SEQUENCE_END_cb(RSA, RSAPrivateKey)


ASN1_SEQUENCE_cb(RSAPublicKey, rsa_cb) = {
	ASN1_SIMPLE(RSA, n, BIGNUM),
	ASN1_SIMPLE(RSA, e, BIGNUM),
} ASN1_SEQUENCE_END_cb(RSA, RSAPublicKey)

ASN1_SEQUENCE(RSA_PSS_PARAMS) = {
	ASN1_EXP_OPT(RSA_PSS_PARAMS, hashAlgorithm, X509_ALGOR,0),
	ASN1_EXP_OPT(RSA_PSS_PARAMS, maskGenAlgorithm, X509_ALGOR,1),
	ASN1_EXP_OPT(RSA_PSS_PARAMS, saltLength, ASN1_INTEGER,2),
	ASN1_EXP_OPT(RSA_PSS_PARAMS, trailerField, ASN1_INTEGER,3)
} ASN1_SEQUENCE_END(RSA_PSS_PARAMS)

IMPLEMENT_ASN1_FUNCTIONS(RSA_PSS_PARAMS)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(RSA, RSAPrivateKey, RSAPrivateKey)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(RSA, RSAPublicKey, RSAPublicKey)

RSA *RSAPublicKey_dup(RSA *rsa)
	{
	return ASN1_item_dup(ASN1_ITEM_rptr(RSAPublicKey), rsa);
	}

RSA *RSAPrivateKey_dup(RSA *rsa)
	{
	return ASN1_item_dup(ASN1_ITEM_rptr(RSAPrivateKey), rsa);
	}