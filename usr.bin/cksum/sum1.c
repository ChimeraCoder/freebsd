
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
#if 0
static char sccsid[] = "@(#)sum1.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <unistd.h>
#include <stdint.h>

#include "extern.h"

int
csum1(int fd, uint32_t *cval, off_t *clen)
{
	int nr;
	u_int lcrc;
	off_t total;
	u_char *p;
	u_char buf[8192];

	/*
	 * 16-bit checksum, rotating right before each addition;
	 * overflow is discarded.
	 */
	lcrc = total = 0;
	while ((nr = read(fd, buf, sizeof(buf))) > 0)
		for (total += nr, p = buf; nr--; ++p) {
			if (lcrc & 1)
				lcrc |= 0x10000;
			lcrc = ((lcrc >> 1) + *p) & 0xffff;
		}
	if (nr < 0)
		return (1);

	*cval = lcrc;
	*clen = total;
	return (0);
}