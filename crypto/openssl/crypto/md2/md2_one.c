
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
#include <openssl/md2.h>

/* This is a separate file so that #defines in cryptlib.h can
 * map my MD functions to different names */

unsigned char *MD2(const unsigned char *d, size_t n, unsigned char *md)
	{
	MD2_CTX c;
	static unsigned char m[MD2_DIGEST_LENGTH];

	if (md == NULL) md=m;
	if (!MD2_Init(&c))
		return NULL;
#ifndef CHARSET_EBCDIC
	MD2_Update(&c,d,n);
#else
	{
		char temp[1024];
		unsigned long chunk;

		while (n > 0)
		{
			chunk = (n > sizeof(temp)) ? sizeof(temp) : n;
			ebcdic2ascii(temp, d, chunk);
			MD2_Update(&c,temp,chunk);
			n -= chunk;
			d += chunk;
		}
	}
#endif
	MD2_Final(md,&c);
	OPENSSL_cleanse(&c,sizeof(c));	/* Security consideration */
	return(md);
	}