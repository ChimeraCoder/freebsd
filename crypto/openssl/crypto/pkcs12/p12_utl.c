
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

/* Cheap and nasty Unicode stuff */

unsigned char *OPENSSL_asc2uni(const char *asc, int asclen, unsigned char **uni, int *unilen)
{
	int ulen, i;
	unsigned char *unitmp;
	if (asclen == -1) asclen = strlen(asc);
	ulen = asclen*2  + 2;
	if (!(unitmp = OPENSSL_malloc(ulen))) return NULL;
	for (i = 0; i < ulen - 2; i+=2) {
		unitmp[i] = 0;
		unitmp[i + 1] = asc[i>>1];
	}
	/* Make result double null terminated */
	unitmp[ulen - 2] = 0;
	unitmp[ulen - 1] = 0;
	if (unilen) *unilen = ulen;
	if (uni) *uni = unitmp;
	return unitmp;
}

char *OPENSSL_uni2asc(unsigned char *uni, int unilen)
{
	int asclen, i;
	char *asctmp;
	asclen = unilen / 2;
	/* If no terminating zero allow for one */
	if (!unilen || uni[unilen - 1]) asclen++;
	uni++;
	if (!(asctmp = OPENSSL_malloc(asclen))) return NULL;
	for (i = 0; i < unilen; i+=2) asctmp[i>>1] = uni[i];
	asctmp[asclen - 1] = 0;
	return asctmp;
}

int i2d_PKCS12_bio(BIO *bp, PKCS12 *p12)
{
	return ASN1_item_i2d_bio(ASN1_ITEM_rptr(PKCS12), bp, p12);
}

#ifndef OPENSSL_NO_FP_API
int i2d_PKCS12_fp(FILE *fp, PKCS12 *p12)
{
	return ASN1_item_i2d_fp(ASN1_ITEM_rptr(PKCS12), fp, p12);
}
#endif

PKCS12 *d2i_PKCS12_bio(BIO *bp, PKCS12 **p12)
{
	return ASN1_item_d2i_bio(ASN1_ITEM_rptr(PKCS12), bp, p12);
}
#ifndef OPENSSL_NO_FP_API
PKCS12 *d2i_PKCS12_fp(FILE *fp, PKCS12 **p12)
{
        return ASN1_item_d2i_fp(ASN1_ITEM_rptr(PKCS12), fp, p12);
}
#endif

PKCS12_SAFEBAG *PKCS12_x5092certbag(X509 *x509)
{
	return PKCS12_item_pack_safebag(x509, ASN1_ITEM_rptr(X509),
			NID_x509Certificate, NID_certBag);
}

PKCS12_SAFEBAG *PKCS12_x509crl2certbag(X509_CRL *crl)
{
	return PKCS12_item_pack_safebag(crl, ASN1_ITEM_rptr(X509_CRL),
			NID_x509Crl, NID_crlBag);
}

X509 *PKCS12_certbag2x509(PKCS12_SAFEBAG *bag)
{
	if(M_PKCS12_bag_type(bag) != NID_certBag) return NULL;
	if(M_PKCS12_cert_bag_type(bag) != NID_x509Certificate) return NULL;
	return ASN1_item_unpack(bag->value.bag->value.octet, ASN1_ITEM_rptr(X509));
}

X509_CRL *PKCS12_certbag2x509crl(PKCS12_SAFEBAG *bag)
{
	if(M_PKCS12_bag_type(bag) != NID_crlBag) return NULL;
	if(M_PKCS12_cert_bag_type(bag) != NID_x509Crl) return NULL;
	return ASN1_item_unpack(bag->value.bag->value.octet,
							ASN1_ITEM_rptr(X509_CRL));
}