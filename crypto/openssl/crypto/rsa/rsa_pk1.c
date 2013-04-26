
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
#include <openssl/rand.h>

int RSA_padding_add_PKCS1_type_1(unsigned char *to, int tlen,
	     const unsigned char *from, int flen)
	{
	int j;
	unsigned char *p;

	if (flen > (tlen-RSA_PKCS1_PADDING_SIZE))
		{
		RSAerr(RSA_F_RSA_PADDING_ADD_PKCS1_TYPE_1,RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE);
		return(0);
		}
	
	p=(unsigned char *)to;

	*(p++)=0;
	*(p++)=1; /* Private Key BT (Block Type) */

	/* pad out with 0xff data */
	j=tlen-3-flen;
	memset(p,0xff,j);
	p+=j;
	*(p++)='\0';
	memcpy(p,from,(unsigned int)flen);
	return(1);
	}

int RSA_padding_check_PKCS1_type_1(unsigned char *to, int tlen,
	     const unsigned char *from, int flen, int num)
	{
	int i,j;
	const unsigned char *p;

	p=from;
	if ((num != (flen+1)) || (*(p++) != 01))
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,RSA_R_BLOCK_TYPE_IS_NOT_01);
		return(-1);
		}

	/* scan over padding data */
	j=flen-1; /* one for type. */
	for (i=0; i<j; i++)
		{
		if (*p != 0xff) /* should decrypt to 0xff */
			{
			if (*p == 0)
				{ p++; break; }
			else	{
				RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,RSA_R_BAD_FIXED_HEADER_DECRYPT);
				return(-1);
				}
			}
		p++;
		}

	if (i == j)
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,RSA_R_NULL_BEFORE_BLOCK_MISSING);
		return(-1);
		}

	if (i < 8)
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,RSA_R_BAD_PAD_BYTE_COUNT);
		return(-1);
		}
	i++; /* Skip over the '\0' */
	j-=i;
	if (j > tlen)
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,RSA_R_DATA_TOO_LARGE);
		return(-1);
		}
	memcpy(to,p,(unsigned int)j);

	return(j);
	}

int RSA_padding_add_PKCS1_type_2(unsigned char *to, int tlen,
	     const unsigned char *from, int flen)
	{
	int i,j;
	unsigned char *p;
	
	if (flen > (tlen-11))
		{
		RSAerr(RSA_F_RSA_PADDING_ADD_PKCS1_TYPE_2,RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE);
		return(0);
		}
	
	p=(unsigned char *)to;

	*(p++)=0;
	*(p++)=2; /* Public Key BT (Block Type) */

	/* pad out with non-zero random data */
	j=tlen-3-flen;

	if (RAND_bytes(p,j) <= 0)
		return(0);
	for (i=0; i<j; i++)
		{
		if (*p == '\0')
			do	{
				if (RAND_bytes(p,1) <= 0)
					return(0);
				} while (*p == '\0');
		p++;
		}

	*(p++)='\0';

	memcpy(p,from,(unsigned int)flen);
	return(1);
	}

int RSA_padding_check_PKCS1_type_2(unsigned char *to, int tlen,
	     const unsigned char *from, int flen, int num)
	{
	int i,j;
	const unsigned char *p;

	p=from;
	if ((num != (flen+1)) || (*(p++) != 02))
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,RSA_R_BLOCK_TYPE_IS_NOT_02);
		return(-1);
		}
#ifdef PKCS1_CHECK
	return(num-11);
#endif

	/* scan over padding data */
	j=flen-1; /* one for type. */
	for (i=0; i<j; i++)
		if (*(p++) == 0) break;

	if (i == j)
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,RSA_R_NULL_BEFORE_BLOCK_MISSING);
		return(-1);
		}

	if (i < 8)
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,RSA_R_BAD_PAD_BYTE_COUNT);
		return(-1);
		}
	i++; /* Skip over the '\0' */
	j-=i;
	if (j > tlen)
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,RSA_R_DATA_TOO_LARGE);
		return(-1);
		}
	memcpy(to,p,(unsigned int)j);

	return(j);
	}