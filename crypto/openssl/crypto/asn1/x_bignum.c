
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
#include <openssl/bn.h>

/* Custom primitive type for BIGNUM handling. This reads in an ASN1_INTEGER as a
 * BIGNUM directly. Currently it ignores the sign which isn't a problem since all
 * BIGNUMs used are non negative and anything that looks negative is normally due
 * to an encoding error.
 */

#define BN_SENSITIVE	1

static int bn_new(ASN1_VALUE **pval, const ASN1_ITEM *it);
static void bn_free(ASN1_VALUE **pval, const ASN1_ITEM *it);

static int bn_i2c(ASN1_VALUE **pval, unsigned char *cont, int *putype, const ASN1_ITEM *it);
static int bn_c2i(ASN1_VALUE **pval, const unsigned char *cont, int len, int utype, char *free_cont, const ASN1_ITEM *it);

static ASN1_PRIMITIVE_FUNCS bignum_pf = {
	NULL, 0,
	bn_new,
	bn_free,
	0,
	bn_c2i,
	bn_i2c
};

ASN1_ITEM_start(BIGNUM)
	ASN1_ITYPE_PRIMITIVE, V_ASN1_INTEGER, NULL, 0, &bignum_pf, 0, "BIGNUM"
ASN1_ITEM_end(BIGNUM)

ASN1_ITEM_start(CBIGNUM)
	ASN1_ITYPE_PRIMITIVE, V_ASN1_INTEGER, NULL, 0, &bignum_pf, BN_SENSITIVE, "BIGNUM"
ASN1_ITEM_end(CBIGNUM)

static int bn_new(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
	*pval = (ASN1_VALUE *)BN_new();
	if(*pval) return 1;
	else return 0;
}

static void bn_free(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
	if(!*pval) return;
	if(it->size & BN_SENSITIVE) BN_clear_free((BIGNUM *)*pval);
	else BN_free((BIGNUM *)*pval);
	*pval = NULL;
}

static int bn_i2c(ASN1_VALUE **pval, unsigned char *cont, int *putype, const ASN1_ITEM *it)
{
	BIGNUM *bn;
	int pad;
	if(!*pval) return -1;
	bn = (BIGNUM *)*pval;
	/* If MSB set in an octet we need a padding byte */
	if(BN_num_bits(bn) & 0x7) pad = 0;
	else pad = 1;
	if(cont) {
		if(pad) *cont++ = 0;
		BN_bn2bin(bn, cont);
	}
	return pad + BN_num_bytes(bn);
}

static int bn_c2i(ASN1_VALUE **pval, const unsigned char *cont, int len,
		  int utype, char *free_cont, const ASN1_ITEM *it)
{
	BIGNUM *bn;
	if(!*pval) bn_new(pval, it);
	bn  = (BIGNUM *)*pval;
	if(!BN_bin2bn(cont, len, bn)) {
		bn_free(pval, it);
		return 0;
	}
	return 1;
}