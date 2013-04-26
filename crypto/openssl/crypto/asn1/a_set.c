
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
#include <openssl/asn1_mac.h>

#ifndef NO_ASN1_OLD

typedef struct
    {
    unsigned char *pbData;
    int cbData;
    } MYBLOB;

/* SetBlobCmp
 * This function compares two elements of SET_OF block
 */
static int SetBlobCmp(const void *elem1, const void *elem2 )
    {
    const MYBLOB *b1 = (const MYBLOB *)elem1;
    const MYBLOB *b2 = (const MYBLOB *)elem2;
    int r;

    r = memcmp(b1->pbData, b2->pbData,
	       b1->cbData < b2->cbData ? b1->cbData : b2->cbData);
    if(r != 0)
	return r;
    return b1->cbData-b2->cbData;
    }

/* int is_set:  if TRUE, then sort the contents (i.e. it isn't a SEQUENCE)    */
int i2d_ASN1_SET(STACK_OF(OPENSSL_BLOCK) *a, unsigned char **pp,
		 i2d_of_void *i2d, int ex_tag, int ex_class,
		 int is_set)
	{
	int ret=0,r;
	int i;
	unsigned char *p;
        unsigned char *pStart, *pTempMem;
        MYBLOB *rgSetBlob;
        int totSize;

	if (a == NULL) return(0);
	for (i=sk_OPENSSL_BLOCK_num(a)-1; i>=0; i--)
		ret+=i2d(sk_OPENSSL_BLOCK_value(a,i),NULL);
	r=ASN1_object_size(1,ret,ex_tag);
	if (pp == NULL) return(r);

	p= *pp;
	ASN1_put_object(&p,1,ret,ex_tag,ex_class);

/* Modified by gp@nsj.co.jp */
	/* And then again by Ben */
	/* And again by Steve */

	if(!is_set || (sk_OPENSSL_BLOCK_num(a) < 2))
		{
		for (i=0; i<sk_OPENSSL_BLOCK_num(a); i++)
                	i2d(sk_OPENSSL_BLOCK_value(a,i),&p);

		*pp=p;
		return(r);
		}

        pStart  = p; /* Catch the beg of Setblobs*/
		/* In this array we will store the SET blobs */
		rgSetBlob = OPENSSL_malloc(sk_OPENSSL_BLOCK_num(a) * sizeof(MYBLOB));
		if (rgSetBlob == NULL)
			{
			ASN1err(ASN1_F_I2D_ASN1_SET,ERR_R_MALLOC_FAILURE);
			return(0);
			}

        for (i=0; i<sk_OPENSSL_BLOCK_num(a); i++)
	        {
                rgSetBlob[i].pbData = p;  /* catch each set encode blob */
                i2d(sk_OPENSSL_BLOCK_value(a,i),&p);
                rgSetBlob[i].cbData = p - rgSetBlob[i].pbData; /* Length of this
SetBlob
*/
		}
        *pp=p;
        totSize = p - pStart; /* This is the total size of all set blobs */

 /* Now we have to sort the blobs. I am using a simple algo.
    *Sort ptrs *Copy to temp-mem *Copy from temp-mem to user-mem*/
        qsort( rgSetBlob, sk_OPENSSL_BLOCK_num(a), sizeof(MYBLOB), SetBlobCmp);
		if (!(pTempMem = OPENSSL_malloc(totSize)))
			{
			ASN1err(ASN1_F_I2D_ASN1_SET,ERR_R_MALLOC_FAILURE);
			return(0);
			}

/* Copy to temp mem */
        p = pTempMem;
        for(i=0; i<sk_OPENSSL_BLOCK_num(a); ++i)
		{
                memcpy(p, rgSetBlob[i].pbData, rgSetBlob[i].cbData);
                p += rgSetBlob[i].cbData;
		}

/* Copy back to user mem*/
        memcpy(pStart, pTempMem, totSize);
        OPENSSL_free(pTempMem);
        OPENSSL_free(rgSetBlob);

        return(r);
        }

STACK_OF(OPENSSL_BLOCK) *d2i_ASN1_SET(STACK_OF(OPENSSL_BLOCK) **a,
			      const unsigned char **pp,
			      long length, d2i_of_void *d2i,
			      void (*free_func)(OPENSSL_BLOCK), int ex_tag,
			      int ex_class)
	{
	ASN1_const_CTX c;
	STACK_OF(OPENSSL_BLOCK) *ret=NULL;

	if ((a == NULL) || ((*a) == NULL))
		{
		if ((ret=sk_OPENSSL_BLOCK_new_null()) == NULL)
			{
			ASN1err(ASN1_F_D2I_ASN1_SET,ERR_R_MALLOC_FAILURE);
			goto err;
			}
		}
	else
		ret=(*a);

	c.p= *pp;
	c.max=(length == 0)?0:(c.p+length);

	c.inf=ASN1_get_object(&c.p,&c.slen,&c.tag,&c.xclass,c.max-c.p);
	if (c.inf & 0x80) goto err;
	if (ex_class != c.xclass)
		{
		ASN1err(ASN1_F_D2I_ASN1_SET,ASN1_R_BAD_CLASS);
		goto err;
		}
	if (ex_tag != c.tag)
		{
		ASN1err(ASN1_F_D2I_ASN1_SET,ASN1_R_BAD_TAG);
		goto err;
		}
	if ((c.slen+c.p) > c.max)
		{
		ASN1err(ASN1_F_D2I_ASN1_SET,ASN1_R_LENGTH_ERROR);
		goto err;
		}
	/* check for infinite constructed - it can be as long
	 * as the amount of data passed to us */
	if (c.inf == (V_ASN1_CONSTRUCTED+1))
		c.slen=length+ *pp-c.p;
	c.max=c.p+c.slen;

	while (c.p < c.max)
		{
		char *s;

		if (M_ASN1_D2I_end_sequence()) break;
		/* XXX: This was called with 4 arguments, incorrectly, it seems
		   if ((s=func(NULL,&c.p,c.slen,c.max-c.p)) == NULL) */
		if ((s=d2i(NULL,&c.p,c.slen)) == NULL)
			{
			ASN1err(ASN1_F_D2I_ASN1_SET,ASN1_R_ERROR_PARSING_SET_ELEMENT);
			asn1_add_error(*pp,(int)(c.p- *pp));
			goto err;
			}
		if (!sk_OPENSSL_BLOCK_push(ret,s)) goto err;
		}
	if (a != NULL) (*a)=ret;
	*pp=c.p;
	return(ret);
err:
	if ((ret != NULL) && ((a == NULL) || (*a != ret)))
		{
		if (free_func != NULL)
			sk_OPENSSL_BLOCK_pop_free(ret,free_func);
		else
			sk_OPENSSL_BLOCK_free(ret);
		}
	return(NULL);
	}

#endif