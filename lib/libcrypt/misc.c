
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

#include "crypt.h"

static char itoa64[] =		/* 0 ... 63 => ascii - 64 */
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void
_crypt_to64(char *s, u_long v, int n)
{
	while (--n >= 0) {
		*s++ = itoa64[v&0x3f];
		v >>= 6;
	}
}

void
b64_from_24bit(uint8_t B2, uint8_t B1, uint8_t B0, int n, int *buflen, char **cp)
{
	uint32_t w;
	int i;

	w = (B2 << 16) | (B1 << 8) | B0;
	for (i = 0; i < n; i++) {
		**cp = itoa64[w&0x3f];
		(*cp)++;
		if ((*buflen)-- < 0)
			break;
		w >>= 6;
	}
}