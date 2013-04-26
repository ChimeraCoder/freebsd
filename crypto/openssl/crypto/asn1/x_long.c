
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

/* Custom primitive type for long handling. This converts between an ASN1_INTEGER
 * and a long directly.
 */


static int long_new(ASN1_VALUE **pval, const ASN1_ITEM *it);
static void long_free(ASN1_VALUE **pval, const ASN1_ITEM *it);

static int long_i2c(ASN1_VALUE **pval, unsigned char *cont, int *putype, const ASN1_ITEM *it);
static int long_c2i(ASN1_VALUE **pval, const unsigned char *cont, int len, int utype, char *free_cont, const ASN1_ITEM *it);
static int long_print(BIO *out, ASN1_VALUE **pval, const ASN1_ITEM *it, int indent, const ASN1_PCTX *pctx);

static ASN1_PRIMITIVE_FUNCS long_pf = {
	NULL, 0,
	long_new,
	long_free,
	long_free,	/* Clear should set to initial value */
	long_c2i,
	long_i2c,
	long_print
};

ASN1_ITEM_start(LONG)
	ASN1_ITYPE_PRIMITIVE, V_ASN1_INTEGER, NULL, 0, &long_pf, ASN1_LONG_UNDEF, "LONG"
ASN1_ITEM_end(LONG)

ASN1_ITEM_start(ZLONG)
	ASN1_ITYPE_PRIMITIVE, V_ASN1_INTEGER, NULL, 0, &long_pf, 0, "ZLONG"
ASN1_ITEM_end(ZLONG)

static int long_new(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
	*(long *)pval = it->size;
	return 1;
}

static void long_free(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
	*(long *)pval = it->size;
}

static int long_i2c(ASN1_VALUE **pval, unsigned char *cont, int *putype, const ASN1_ITEM *it)
{
	long ltmp;
	unsigned long utmp;
	int clen, pad, i;
	/* this exists to bypass broken gcc optimization */
	char *cp = (char *)pval;

	/* use memcpy, because we may not be long aligned */
	memcpy(&ltmp, cp, sizeof(long));

	if(ltmp == it->size) return -1;
	/* Convert the long to positive: we subtract one if negative so
	 * we can cleanly handle the padding if only the MSB of the leading
	 * octet is set. 
	 */
	if(ltmp < 0) utmp = -ltmp - 1;
	else utmp = ltmp;
	clen = BN_num_bits_word(utmp);
	/* If MSB of leading octet set we need to pad */
	if(!(clen & 0x7)) pad = 1;
	else pad = 0;

	/* Convert number of bits to number of octets */
	clen = (clen + 7) >> 3;

	if(cont) {
		if(pad) *cont++ = (ltmp < 0) ? 0xff : 0;
		for(i = clen - 1; i >= 0; i--) {
			cont[i] = (unsigned char)(utmp & 0xff);
			if(ltmp < 0) cont[i] ^= 0xff;
			utmp >>= 8;
		}
	}
	return clen + pad;
}

static int long_c2i(ASN1_VALUE **pval, const unsigned char *cont, int len,
		    int utype, char *free_cont, const ASN1_ITEM *it)
{
	int neg, i;
	long ltmp;
	unsigned long utmp = 0;
	char *cp = (char *)pval;
	if(len > (int)sizeof(long)) {
		ASN1err(ASN1_F_LONG_C2I, ASN1_R_INTEGER_TOO_LARGE_FOR_LONG);
		return 0;
	}
	/* Is it negative? */
	if(len && (cont[0] & 0x80)) neg = 1;
	else neg = 0;
	utmp = 0;
	for(i = 0; i < len; i++) {
		utmp <<= 8;
		if(neg) utmp |= cont[i] ^ 0xff;
		else utmp |= cont[i];
	}
	ltmp = (long)utmp;
	if(neg) {
		ltmp++;
		ltmp = -ltmp;
	}
	if(ltmp == it->size) {
		ASN1err(ASN1_F_LONG_C2I, ASN1_R_INTEGER_TOO_LARGE_FOR_LONG);
		return 0;
	}
	memcpy(cp, &ltmp, sizeof(long));
	return 1;
}

static int long_print(BIO *out, ASN1_VALUE **pval, const ASN1_ITEM *it,
			int indent, const ASN1_PCTX *pctx)
	{
	return BIO_printf(out, "%ld\n", *(long *)pval);
	}