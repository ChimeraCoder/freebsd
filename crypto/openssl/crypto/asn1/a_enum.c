
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
#include <openssl/bn.h>

/* 
 * Code for ENUMERATED type: identical to INTEGER apart from a different tag.
 * for comments on encoding see a_int.c
 */

int ASN1_ENUMERATED_set(ASN1_ENUMERATED *a, long v)
	{
	int j,k;
	unsigned int i;
	unsigned char buf[sizeof(long)+1];
	long d;

	a->type=V_ASN1_ENUMERATED;
	if (a->length < (int)(sizeof(long)+1))
		{
		if (a->data != NULL)
			OPENSSL_free(a->data);
		if ((a->data=(unsigned char *)OPENSSL_malloc(sizeof(long)+1)) != NULL)
			memset((char *)a->data,0,sizeof(long)+1);
		}
	if (a->data == NULL)
		{
		ASN1err(ASN1_F_ASN1_ENUMERATED_SET,ERR_R_MALLOC_FAILURE);
		return(0);
		}
	d=v;
	if (d < 0)
		{
		d= -d;
		a->type=V_ASN1_NEG_ENUMERATED;
		}

	for (i=0; i<sizeof(long); i++)
		{
		if (d == 0) break;
		buf[i]=(int)d&0xff;
		d>>=8;
		}
	j=0;
	for (k=i-1; k >=0; k--)
		a->data[j++]=buf[k];
	a->length=j;
	return(1);
	}

long ASN1_ENUMERATED_get(ASN1_ENUMERATED *a)
	{
	int neg=0,i;
	long r=0;

	if (a == NULL) return(0L);
	i=a->type;
	if (i == V_ASN1_NEG_ENUMERATED)
		neg=1;
	else if (i != V_ASN1_ENUMERATED)
		return -1;
	
	if (a->length > (int)sizeof(long))
		{
		/* hmm... a bit ugly */
		return(0xffffffffL);
		}
	if (a->data == NULL)
		return 0;

	for (i=0; i<a->length; i++)
		{
		r<<=8;
		r|=(unsigned char)a->data[i];
		}
	if (neg) r= -r;
	return(r);
	}

ASN1_ENUMERATED *BN_to_ASN1_ENUMERATED(BIGNUM *bn, ASN1_ENUMERATED *ai)
	{
	ASN1_ENUMERATED *ret;
	int len,j;

	if (ai == NULL)
		ret=M_ASN1_ENUMERATED_new();
	else
		ret=ai;
	if (ret == NULL)
		{
		ASN1err(ASN1_F_BN_TO_ASN1_ENUMERATED,ERR_R_NESTED_ASN1_ERROR);
		goto err;
		}
	if(BN_is_negative(bn)) ret->type = V_ASN1_NEG_ENUMERATED;
	else ret->type=V_ASN1_ENUMERATED;
	j=BN_num_bits(bn);
	len=((j == 0)?0:((j/8)+1));
	if (ret->length < len+4)
		{
		unsigned char *new_data=OPENSSL_realloc(ret->data, len+4);
		if (!new_data)
			{
			ASN1err(ASN1_F_BN_TO_ASN1_ENUMERATED,ERR_R_MALLOC_FAILURE);
			goto err;
			}
		ret->data=new_data;
		}

	ret->length=BN_bn2bin(bn,ret->data);
	return(ret);
err:
	if (ret != ai) M_ASN1_ENUMERATED_free(ret);
	return(NULL);
	}

BIGNUM *ASN1_ENUMERATED_to_BN(ASN1_ENUMERATED *ai, BIGNUM *bn)
	{
	BIGNUM *ret;

	if ((ret=BN_bin2bn(ai->data,ai->length,bn)) == NULL)
		ASN1err(ASN1_F_ASN1_ENUMERATED_TO_BN,ASN1_R_BN_LIB);
	else if(ai->type == V_ASN1_NEG_ENUMERATED) BN_set_negative(ret,1);
	return(ret);
	}