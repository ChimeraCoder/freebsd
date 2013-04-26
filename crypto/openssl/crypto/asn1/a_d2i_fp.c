
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
#include <limits.h>
#include "cryptlib.h"
#include <openssl/buffer.h>
#include <openssl/asn1_mac.h>

static int asn1_d2i_read_bio(BIO *in, BUF_MEM **pb);

#ifndef NO_OLD_ASN1
#ifndef OPENSSL_NO_FP_API

void *ASN1_d2i_fp(void *(*xnew)(void), d2i_of_void *d2i, FILE *in, void **x)
        {
        BIO *b;
        void *ret;

        if ((b=BIO_new(BIO_s_file())) == NULL)
		{
		ASN1err(ASN1_F_ASN1_D2I_FP,ERR_R_BUF_LIB);
                return(NULL);
		}
        BIO_set_fp(b,in,BIO_NOCLOSE);
        ret=ASN1_d2i_bio(xnew,d2i,b,x);
        BIO_free(b);
        return(ret);
        }
#endif

void *ASN1_d2i_bio(void *(*xnew)(void), d2i_of_void *d2i, BIO *in, void **x)
	{
	BUF_MEM *b = NULL;
	const unsigned char *p;
	void *ret=NULL;
	int len;

	len = asn1_d2i_read_bio(in, &b);
	if(len < 0) goto err;

	p=(unsigned char *)b->data;
	ret=d2i(x,&p,len);
err:
	if (b != NULL) BUF_MEM_free(b);
	return(ret);
	}

#endif

void *ASN1_item_d2i_bio(const ASN1_ITEM *it, BIO *in, void *x)
	{
	BUF_MEM *b = NULL;
	const unsigned char *p;
	void *ret=NULL;
	int len;

	len = asn1_d2i_read_bio(in, &b);
	if(len < 0) goto err;

	p=(const unsigned char *)b->data;
	ret=ASN1_item_d2i(x,&p,len, it);
err:
	if (b != NULL) BUF_MEM_free(b);
	return(ret);
	}

#ifndef OPENSSL_NO_FP_API
void *ASN1_item_d2i_fp(const ASN1_ITEM *it, FILE *in, void *x)
        {
        BIO *b;
        char *ret;

        if ((b=BIO_new(BIO_s_file())) == NULL)
		{
		ASN1err(ASN1_F_ASN1_ITEM_D2I_FP,ERR_R_BUF_LIB);
                return(NULL);
		}
        BIO_set_fp(b,in,BIO_NOCLOSE);
        ret=ASN1_item_d2i_bio(it,b,x);
        BIO_free(b);
        return(ret);
        }
#endif

#define HEADER_SIZE   8
static int asn1_d2i_read_bio(BIO *in, BUF_MEM **pb)
	{
	BUF_MEM *b;
	unsigned char *p;
	int i;
	ASN1_const_CTX c;
	size_t want=HEADER_SIZE;
	int eos=0;
	size_t off=0;
	size_t len=0;

	b=BUF_MEM_new();
	if (b == NULL)
		{
		ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ERR_R_MALLOC_FAILURE);
		return -1;
		}

	ERR_clear_error();
	for (;;)
		{
		if (want >= (len-off))
			{
			want-=(len-off);

			if (len + want < len || !BUF_MEM_grow_clean(b,len+want))
				{
				ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ERR_R_MALLOC_FAILURE);
				goto err;
				}
			i=BIO_read(in,&(b->data[len]),want);
			if ((i < 0) && ((len-off) == 0))
				{
				ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ASN1_R_NOT_ENOUGH_DATA);
				goto err;
				}
			if (i > 0)
				{
				if (len+i < len)
					{
					ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ASN1_R_TOO_LONG);
					goto err;
					}
				len+=i;
				}
			}
		/* else data already loaded */

		p=(unsigned char *)&(b->data[off]);
		c.p=p;
		c.inf=ASN1_get_object(&(c.p),&(c.slen),&(c.tag),&(c.xclass),
			len-off);
		if (c.inf & 0x80)
			{
			unsigned long e;

			e=ERR_GET_REASON(ERR_peek_error());
			if (e != ASN1_R_TOO_LONG)
				goto err;
			else
				ERR_clear_error(); /* clear error */
			}
		i=c.p-p;/* header length */
		off+=i;	/* end of data */

		if (c.inf & 1)
			{
			/* no data body so go round again */
			eos++;
			if (eos < 0)
				{
				ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ASN1_R_HEADER_TOO_LONG);
				goto err;
				}
			want=HEADER_SIZE;
			}
		else if (eos && (c.slen == 0) && (c.tag == V_ASN1_EOC))
			{
			/* eos value, so go back and read another header */
			eos--;
			if (eos <= 0)
				break;
			else
				want=HEADER_SIZE;
			}
		else 
			{
			/* suck in c.slen bytes of data */
			want=c.slen;
			if (want > (len-off))
				{
				want-=(len-off);
				if (want > INT_MAX /* BIO_read takes an int length */ ||
					len+want < len)
						{
						ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ASN1_R_TOO_LONG);
						goto err;
						}
				if (!BUF_MEM_grow_clean(b,len+want))
					{
					ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ERR_R_MALLOC_FAILURE);
					goto err;
					}
				while (want > 0)
					{
					i=BIO_read(in,&(b->data[len]),want);
					if (i <= 0)
						{
						ASN1err(ASN1_F_ASN1_D2I_READ_BIO,
						    ASN1_R_NOT_ENOUGH_DATA);
						goto err;
						}
					/* This can't overflow because
					 * |len+want| didn't overflow. */
					len+=i;
					want-=i;
					}
				}
			if (off + c.slen < off)
				{
				ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ASN1_R_TOO_LONG);
				goto err;
				}
			off+=c.slen;
			if (eos <= 0)
				{
				break;
				}
			else
				want=HEADER_SIZE;
			}
		}

	if (off > INT_MAX)
		{
		ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ASN1_R_TOO_LONG);
		goto err;
		}

	*pb = b;
	return off;
err:
	if (b != NULL) BUF_MEM_free(b);
	return -1;
	}