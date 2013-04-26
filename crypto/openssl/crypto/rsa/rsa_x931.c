
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
#include <openssl/objects.h>

int RSA_padding_add_X931(unsigned char *to, int tlen,
	     const unsigned char *from, int flen)
	{
	int j;
	unsigned char *p;

	/* Absolute minimum amount of padding is 1 header nibble, 1 padding
	 * nibble and 2 trailer bytes: but 1 hash if is already in 'from'.
	 */

	j = tlen - flen - 2;

	if (j < 0)
		{
		RSAerr(RSA_F_RSA_PADDING_ADD_X931,RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE);
		return -1;
		}
	
	p=(unsigned char *)to;

	/* If no padding start and end nibbles are in one byte */
	if (j == 0)
		*p++ = 0x6A;
	else
		{
		*p++ = 0x6B;
		if (j > 1)
			{
			memset(p, 0xBB, j - 1);
			p += j - 1;
			}
		*p++ = 0xBA;
		}
	memcpy(p,from,(unsigned int)flen);
	p += flen;
	*p = 0xCC;
	return(1);
	}

int RSA_padding_check_X931(unsigned char *to, int tlen,
	     const unsigned char *from, int flen, int num)
	{
	int i = 0,j;
	const unsigned char *p;

	p=from;
	if ((num != flen) || ((*p != 0x6A) && (*p != 0x6B)))
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_X931,RSA_R_INVALID_HEADER);
		return -1;
		}

	if (*p++ == 0x6B)
		{
		j=flen-3;
		for (i = 0; i < j; i++)
			{
			unsigned char c = *p++;
			if (c == 0xBA)
				break;
			if (c != 0xBB)
				{
				RSAerr(RSA_F_RSA_PADDING_CHECK_X931,
					RSA_R_INVALID_PADDING);
				return -1;
				}
			}

		j -= i;

		if (i == 0)
			{
			RSAerr(RSA_F_RSA_PADDING_CHECK_X931, RSA_R_INVALID_PADDING);
			return -1;
			}

		}
	else j = flen - 2;

	if (p[j] != 0xCC)
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_X931, RSA_R_INVALID_TRAILER);
		return -1;
		}

	memcpy(to,p,(unsigned int)j);

	return(j);
	}

/* Translate between X931 hash ids and NIDs */

int RSA_X931_hash_id(int nid)
	{
	switch (nid)
		{
		case NID_sha1:
		return 0x33;

		case NID_sha256:
		return 0x34;

		case NID_sha384:
		return 0x36;

		case NID_sha512:
		return 0x35;

		}
	return -1;
	}