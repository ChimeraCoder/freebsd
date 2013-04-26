
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
#include <openssl/buffer.h>

/* LIMIT_BEFORE_EXPANSION is the maximum n such that (n+3)/3*4 < 2**31. That
 * function is applied in several functions in this file and this limit ensures
 * that the result fits in an int. */
#define LIMIT_BEFORE_EXPANSION 0x5ffffffc

BUF_MEM *BUF_MEM_new(void)
	{
	BUF_MEM *ret;

	ret=OPENSSL_malloc(sizeof(BUF_MEM));
	if (ret == NULL)
		{
		BUFerr(BUF_F_BUF_MEM_NEW,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	ret->length=0;
	ret->max=0;
	ret->data=NULL;
	return(ret);
	}

void BUF_MEM_free(BUF_MEM *a)
	{
	if(a == NULL)
	    return;

	if (a->data != NULL)
		{
		memset(a->data,0,(unsigned int)a->max);
		OPENSSL_free(a->data);
		}
	OPENSSL_free(a);
	}

int BUF_MEM_grow(BUF_MEM *str, size_t len)
	{
	char *ret;
	size_t n;

	if (str->length >= len)
		{
		str->length=len;
		return(len);
		}
	if (str->max >= len)
		{
		memset(&str->data[str->length],0,len-str->length);
		str->length=len;
		return(len);
		}
	/* This limit is sufficient to ensure (len+3)/3*4 < 2**31 */
	if (len > LIMIT_BEFORE_EXPANSION)
		{
		BUFerr(BUF_F_BUF_MEM_GROW,ERR_R_MALLOC_FAILURE);
		return 0;
		}
	n=(len+3)/3*4;
	if (str->data == NULL)
		ret=OPENSSL_malloc(n);
	else
		ret=OPENSSL_realloc(str->data,n);
	if (ret == NULL)
		{
		BUFerr(BUF_F_BUF_MEM_GROW,ERR_R_MALLOC_FAILURE);
		len=0;
		}
	else
		{
		str->data=ret;
		str->max=n;
		memset(&str->data[str->length],0,len-str->length);
		str->length=len;
		}
	return(len);
	}

int BUF_MEM_grow_clean(BUF_MEM *str, size_t len)
	{
	char *ret;
	size_t n;

	if (str->length >= len)
		{
		memset(&str->data[len],0,str->length-len);
		str->length=len;
		return(len);
		}
	if (str->max >= len)
		{
		memset(&str->data[str->length],0,len-str->length);
		str->length=len;
		return(len);
		}
	/* This limit is sufficient to ensure (len+3)/3*4 < 2**31 */
	if (len > LIMIT_BEFORE_EXPANSION)
		{
		BUFerr(BUF_F_BUF_MEM_GROW_CLEAN,ERR_R_MALLOC_FAILURE);
		return 0;
		}
	n=(len+3)/3*4;
	if (str->data == NULL)
		ret=OPENSSL_malloc(n);
	else
		ret=OPENSSL_realloc_clean(str->data,str->max,n);
	if (ret == NULL)
		{
		BUFerr(BUF_F_BUF_MEM_GROW_CLEAN,ERR_R_MALLOC_FAILURE);
		len=0;
		}
	else
		{
		str->data=ret;
		str->max=n;
		memset(&str->data[str->length],0,len-str->length);
		str->length=len;
		}
	return(len);
	}

void BUF_reverse(unsigned char *out, unsigned char *in, size_t size)
	{
	size_t i;
	if (in)
		{
		out += size - 1;
		for (i = 0; i < size; i++)
			*in++ = *out--;
		}
	else
		{
		unsigned char *q;
		char c;
		q = out + size - 1;
		for (i = 0; i < size/2; i++)
			{
			c = *q;
			*q-- = *out;
			*out++ = c;
			}
		}
	}