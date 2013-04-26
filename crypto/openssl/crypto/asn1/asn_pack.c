
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

#ifndef NO_ASN1_OLD

/* ASN1 packing and unpacking functions */

/* Turn an ASN1 encoded SEQUENCE OF into a STACK of structures */

STACK_OF(OPENSSL_BLOCK) *ASN1_seq_unpack(const unsigned char *buf, int len,
			 d2i_of_void *d2i, void (*free_func)(OPENSSL_BLOCK))
{
    STACK_OF(OPENSSL_BLOCK) *sk;
    const unsigned char *pbuf;
    pbuf =  buf;
    if (!(sk = d2i_ASN1_SET(NULL, &pbuf, len, d2i, free_func,
					V_ASN1_SEQUENCE, V_ASN1_UNIVERSAL)))
		 ASN1err(ASN1_F_ASN1_SEQ_UNPACK,ASN1_R_DECODE_ERROR);
    return sk;
}

/* Turn a STACK structures into an ASN1 encoded SEQUENCE OF structure in a
 * OPENSSL_malloc'ed buffer
 */

unsigned char *ASN1_seq_pack(STACK_OF(OPENSSL_BLOCK) *safes, i2d_of_void *i2d,
			     unsigned char **buf, int *len)
{
	int safelen;
	unsigned char *safe, *p;
	if (!(safelen = i2d_ASN1_SET(safes, NULL, i2d, V_ASN1_SEQUENCE,
					      V_ASN1_UNIVERSAL, IS_SEQUENCE))) {
		ASN1err(ASN1_F_ASN1_SEQ_PACK,ASN1_R_ENCODE_ERROR);
		return NULL;
	}
	if (!(safe = OPENSSL_malloc (safelen))) {
		ASN1err(ASN1_F_ASN1_SEQ_PACK,ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	p = safe;
	i2d_ASN1_SET(safes, &p, i2d, V_ASN1_SEQUENCE, V_ASN1_UNIVERSAL,
								 IS_SEQUENCE);
	if (len) *len = safelen;
	if (buf) *buf = safe;
	return safe;
}

/* Extract an ASN1 object from an ASN1_STRING */

void *ASN1_unpack_string(ASN1_STRING *oct, d2i_of_void *d2i)
{
	const unsigned char *p;
	char *ret;

	p = oct->data;
	if(!(ret = d2i(NULL, &p, oct->length)))
		ASN1err(ASN1_F_ASN1_UNPACK_STRING,ASN1_R_DECODE_ERROR);
	return ret;
}

/* Pack an ASN1 object into an ASN1_STRING */

ASN1_STRING *ASN1_pack_string(void *obj, i2d_of_void *i2d, ASN1_STRING **oct)
{
	unsigned char *p;
	ASN1_STRING *octmp;

	if (!oct || !*oct) {
		if (!(octmp = ASN1_STRING_new ())) {
			ASN1err(ASN1_F_ASN1_PACK_STRING,ERR_R_MALLOC_FAILURE);
			return NULL;
		}
		if (oct) *oct = octmp;
	} else octmp = *oct;
		
	if (!(octmp->length = i2d(obj, NULL))) {
		ASN1err(ASN1_F_ASN1_PACK_STRING,ASN1_R_ENCODE_ERROR);
		return NULL;
	}
	if (!(p = OPENSSL_malloc (octmp->length))) {
		ASN1err(ASN1_F_ASN1_PACK_STRING,ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	octmp->data = p;
	i2d (obj, &p);
	return octmp;
}

#endif

/* ASN1_ITEM versions of the above */

ASN1_STRING *ASN1_item_pack(void *obj, const ASN1_ITEM *it, ASN1_STRING **oct)
{
	ASN1_STRING *octmp;

	if (!oct || !*oct) {
		if (!(octmp = ASN1_STRING_new ())) {
			ASN1err(ASN1_F_ASN1_ITEM_PACK,ERR_R_MALLOC_FAILURE);
			return NULL;
		}
		if (oct) *oct = octmp;
	} else octmp = *oct;

	if(octmp->data) {
		OPENSSL_free(octmp->data);
		octmp->data = NULL;
	}
		
	if (!(octmp->length = ASN1_item_i2d(obj, &octmp->data, it))) {
		ASN1err(ASN1_F_ASN1_ITEM_PACK,ASN1_R_ENCODE_ERROR);
		return NULL;
	}
	if (!octmp->data) {
		ASN1err(ASN1_F_ASN1_ITEM_PACK,ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	return octmp;
}

/* Extract an ASN1 object from an ASN1_STRING */

void *ASN1_item_unpack(ASN1_STRING *oct, const ASN1_ITEM *it)
{
	const unsigned char *p;
	void *ret;

	p = oct->data;
	if(!(ret = ASN1_item_d2i(NULL, &p, oct->length, it)))
		ASN1err(ASN1_F_ASN1_ITEM_UNPACK,ASN1_R_DECODE_ERROR);
	return ret;
}