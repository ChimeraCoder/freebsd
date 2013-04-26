
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <stdio.h>
#include <string.h>
#include "decode.h"

int decode(FILE *fd, char *buffer, int len)
{
	int n, pos = 0, tpos;
	char *bp, *p;
	char tbuffer[3];
	char temp[128];

#define	DEC(c)	(((c) - ' ') & 0x3f)

	do {
		if (!fgets(temp, sizeof(temp), fd))
			return(0);
	} while (strncmp(temp, "begin ", 6));
	sscanf(temp, "begin %o %s", (unsigned *)&n, temp);
	bp = buffer;
	for (;;) {
		if (!fgets(p = temp, sizeof(temp), fd))
			return(0);
		if ((n = DEC(*p)) <= 0)
			break;
		for (++p; n > 0; p += 4, n -= 3) {
			tpos = 0;
			if (n >= 3) {
				tbuffer[tpos++] = DEC(p[0])<<2 | DEC(p[1])>>4;
				tbuffer[tpos++] = DEC(p[1])<<4 | DEC(p[2])>>2;
				tbuffer[tpos++] = DEC(p[2])<<6 | DEC(p[3]);
			}
			else {
				if (n >= 1) {
					tbuffer[tpos++] =
						DEC(p[0])<<2 | DEC(p[1])>>4;
				}
				if (n >= 2) {
					tbuffer[tpos++] =
						DEC(p[1])<<4 | DEC(p[2])>>2;
				}
				if (n >= 3) {
					tbuffer[tpos++] =
						DEC(p[2])<<6 | DEC(p[3]);
				}
			}
			if (tpos == 0)
				continue;
			if (tpos + pos > len) {
				tpos = len - pos;
				/*
				 * Arrange return value > len to indicate
				 * overflow.
				 */
				pos++;
			}
			bcopy(tbuffer, bp, tpos);
			pos += tpos;
			bp += tpos;
			if (pos > len)
				return(pos);
		}
	}
	if (!fgets(temp, sizeof(temp), fd) || strcmp(temp, "end\n"))
		return(0);
	return(pos);
}