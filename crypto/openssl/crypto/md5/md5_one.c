
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
#include <string.h>
#include <openssl/md5.h>
#include <openssl/crypto.h>

#ifdef CHARSET_EBCDIC
#include <openssl/ebcdic.h>
#endif

unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md)
	{
	MD5_CTX c;
	static unsigned char m[MD5_DIGEST_LENGTH];

	if (md == NULL) md=m;
	if (!MD5_Init(&c))
		return NULL;
#ifndef CHARSET_EBCDIC
	MD5_Update(&c,d,n);
#else
	{
		char temp[1024];
		unsigned long chunk;

		while (n > 0)
		{
			chunk = (n > sizeof(temp)) ? sizeof(temp) : n;
			ebcdic2ascii(temp, d, chunk);
			MD5_Update(&c,temp,chunk);
			n -= chunk;
			d += chunk;
		}
	}
#endif
	MD5_Final(md,&c);
	OPENSSL_cleanse(&c,sizeof(c)); /* security consideration */
	return(md);
	}