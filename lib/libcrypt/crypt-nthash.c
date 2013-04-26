
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <netinet/in.h>

#include <ctype.h>
#include <err.h>
#include <md4.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "crypt.h"

/*
 * NT HASH = md4(str2unicode(pw))
 */

/* ARGSUSED */
char *
crypt_nthash(const char *pw, const char *salt __unused)
{
	size_t unipwLen;
	int i, j;
	static char hexconvtab[] = "0123456789abcdef";
	static const char *magic = "$3$";
	static char passwd[120];
	u_int16_t unipw[128];
	char final[MD4_SIZE*2 + 1];
	u_char hash[MD4_SIZE];
	const char *s;
	MD4_CTX	ctx;
  
	bzero(unipw, sizeof(unipw)); 
	/* convert to unicode (thanx Archie) */
	unipwLen = 0;
	for (s = pw; unipwLen < sizeof(unipw) / 2 && *s; s++)
		unipw[unipwLen++] = htons(*s << 8);
        
	/* Compute MD4 of Unicode password */
 	MD4Init(&ctx);
	MD4Update(&ctx, (u_char *)unipw, unipwLen*sizeof(u_int16_t));
	MD4Final(hash, &ctx);  
	
	for (i = j = 0; i < MD4_SIZE; i++) {
		final[j++] = hexconvtab[hash[i] >> 4];
		final[j++] = hexconvtab[hash[i] & 15];
	}
	final[j] = '\0';

	strcpy(passwd, magic);
	strcat(passwd, "$");
	strncat(passwd, final, MD4_SIZE*2);

	/* Don't leave anything around in vm they could use. */
	memset(final, 0, sizeof(final));

	return (passwd);
}