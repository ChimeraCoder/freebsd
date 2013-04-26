
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
#include <openssl/ripemd.h>
#include <openssl/crypto.h>

unsigned char *RIPEMD160(const unsigned char *d, size_t n,
	     unsigned char *md)
	{
	RIPEMD160_CTX c;
	static unsigned char m[RIPEMD160_DIGEST_LENGTH];

	if (md == NULL) md=m;
	if (!RIPEMD160_Init(&c))
		return NULL;
	RIPEMD160_Update(&c,d,n);
	RIPEMD160_Final(md,&c);
	OPENSSL_cleanse(&c,sizeof(c)); /* security consideration */
	return(md);
	}