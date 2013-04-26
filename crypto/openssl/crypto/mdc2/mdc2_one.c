
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
#include <openssl/mdc2.h>

unsigned char *MDC2(const unsigned char *d, size_t n, unsigned char *md)
	{
	MDC2_CTX c;
	static unsigned char m[MDC2_DIGEST_LENGTH];

	if (md == NULL) md=m;
	if (!MDC2_Init(&c))
		return NULL;
	MDC2_Update(&c,d,n);
        MDC2_Final(md,&c);
	OPENSSL_cleanse(&c,sizeof(c)); /* security consideration */
	return(md);
	}