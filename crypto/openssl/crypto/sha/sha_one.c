
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
#include <openssl/sha.h>
#include <openssl/crypto.h>

#ifndef OPENSSL_NO_SHA0
unsigned char *SHA(const unsigned char *d, size_t n, unsigned char *md)
	{
	SHA_CTX c;
	static unsigned char m[SHA_DIGEST_LENGTH];

	if (md == NULL) md=m;
	if (!SHA_Init(&c))
		return NULL;
	SHA_Update(&c,d,n);
	SHA_Final(md,&c);
	OPENSSL_cleanse(&c,sizeof(c));
	return(md);
	}
#endif