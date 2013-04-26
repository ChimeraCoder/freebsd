
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
#include <openssl/dsa.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/rand.h>

/* Override the default new methods */
static int sig_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
								void *exarg)
{
	if(operation == ASN1_OP_NEW_PRE) {
		DSA_SIG *sig;
		sig = OPENSSL_malloc(sizeof(DSA_SIG));
		if (!sig)
			{
			DSAerr(DSA_F_SIG_CB, ERR_R_MALLOC_FAILURE);
			return 0;
			}
		sig->r = NULL;
		sig->s = NULL;
		*pval = (ASN1_VALUE *)sig;
		return 2;
	}
	return 1;
}

ASN1_SEQUENCE_cb(DSA_SIG, sig_cb) = {
	ASN1_SIMPLE(DSA_SIG, r, CBIGNUM),
	ASN1_SIMPLE(DSA_SIG, s, CBIGNUM)
} ASN1_SEQUENCE_END_cb(DSA_SIG, DSA_SIG)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(DSA_SIG, DSA_SIG, DSA_SIG)

/* Override the default free and new methods */
static int dsa_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
							void *exarg)
{
	if(operation == ASN1_OP_NEW_PRE) {
		*pval = (ASN1_VALUE *)DSA_new();
		if(*pval) return 2;
		return 0;
	} else if(operation == ASN1_OP_FREE_PRE) {
		DSA_free((DSA *)*pval);
		*pval = NULL;
		return 2;
	}
	return 1;
}

ASN1_SEQUENCE_cb(DSAPrivateKey, dsa_cb) = {
	ASN1_SIMPLE(DSA, version, LONG),
	ASN1_SIMPLE(DSA, p, BIGNUM),
	ASN1_SIMPLE(DSA, q, BIGNUM),
	ASN1_SIMPLE(DSA, g, BIGNUM),
	ASN1_SIMPLE(DSA, pub_key, BIGNUM),
	ASN1_SIMPLE(DSA, priv_key, BIGNUM)
} ASN1_SEQUENCE_END_cb(DSA, DSAPrivateKey)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(DSA, DSAPrivateKey, DSAPrivateKey)

ASN1_SEQUENCE_cb(DSAparams, dsa_cb) = {
	ASN1_SIMPLE(DSA, p, BIGNUM),
	ASN1_SIMPLE(DSA, q, BIGNUM),
	ASN1_SIMPLE(DSA, g, BIGNUM),
} ASN1_SEQUENCE_END_cb(DSA, DSAparams)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(DSA, DSAparams, DSAparams)

/* DSA public key is a bit trickier... its effectively a CHOICE type
 * decided by a field called write_params which can either write out
 * just the public key as an INTEGER or the parameters and public key
 * in a SEQUENCE
 */

ASN1_SEQUENCE(dsa_pub_internal) = {
	ASN1_SIMPLE(DSA, pub_key, BIGNUM),
	ASN1_SIMPLE(DSA, p, BIGNUM),
	ASN1_SIMPLE(DSA, q, BIGNUM),
	ASN1_SIMPLE(DSA, g, BIGNUM)
} ASN1_SEQUENCE_END_name(DSA, dsa_pub_internal)

ASN1_CHOICE_cb(DSAPublicKey, dsa_cb) = {
	ASN1_SIMPLE(DSA, pub_key, BIGNUM),
	ASN1_EX_COMBINE(0, 0, dsa_pub_internal)
} ASN1_CHOICE_END_cb(DSA, DSAPublicKey, write_params)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(DSA, DSAPublicKey, DSAPublicKey)

DSA *DSAparams_dup(DSA *dsa)
	{
	return ASN1_item_dup(ASN1_ITEM_rptr(DSAparams), dsa);
	}

int DSA_sign(int type, const unsigned char *dgst, int dlen, unsigned char *sig,
	     unsigned int *siglen, DSA *dsa)
	{
	DSA_SIG *s;
	RAND_seed(dgst, dlen);
	s=DSA_do_sign(dgst,dlen,dsa);
	if (s == NULL)
		{
		*siglen=0;
		return(0);
		}
	*siglen=i2d_DSA_SIG(s,&sig);
	DSA_SIG_free(s);
	return(1);
	}

/* data has already been hashed (probably with SHA or SHA-1). */
/* returns
 *      1: correct signature
 *      0: incorrect signature
 *     -1: error
 */
int DSA_verify(int type, const unsigned char *dgst, int dgst_len,
	     const unsigned char *sigbuf, int siglen, DSA *dsa)
	{
	DSA_SIG *s;
	int ret=-1;

	s = DSA_SIG_new();
	if (s == NULL) return(ret);
	if (d2i_DSA_SIG(&s,&sigbuf,siglen) == NULL) goto err;
	ret=DSA_do_verify(dgst,dgst_len,s,dsa);
err:
	DSA_SIG_free(s);
	return(ret);
	}