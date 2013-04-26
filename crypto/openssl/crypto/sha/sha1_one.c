
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
#include <openssl/crypto.h>
#include <openssl/sha.h>

#ifndef OPENSSL_NO_SHA1
unsigned char *SHA1(const unsigned char *d, size_t n, unsigned char *md)
	{
	SHA_CTX c;
	static unsigned char m[SHA_DIGEST_LENGTH];

	if (md == NULL) md=m;
	if (!SHA1_Init(&c))
		return NULL;
	SHA1_Update(&c,d,n);
	SHA1_Final(md,&c);
	OPENSSL_cleanse(&c,sizeof(c));
	return(md);
	}
#endif