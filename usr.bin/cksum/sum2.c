
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
static char sccsid[] = "@(#)sum2.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <unistd.h>
#include <stdint.h>

#include "extern.h"

int
csum2(int fd, uint32_t *cval, off_t *clen)
{
	uint32_t lcrc;
	int nr;
	off_t total;
	u_char *p;
	u_char buf[8192];

	/*
	 * Draft 8 POSIX 1003.2:
	 *
	 *   s = sum of all bytes
	 *   r = s % 2^16 + (s % 2^32) / 2^16
	 * lcrc = (r % 2^16) + r / 2^16
	 */
	lcrc = total = 0;
	while ((nr = read(fd, buf, sizeof(buf))) > 0)
		for (total += nr, p = buf; nr--; ++p)
			lcrc += *p;
	if (nr < 0)
		return (1);

	lcrc = (lcrc & 0xffff) + (lcrc >> 16);
	lcrc = (lcrc & 0xffff) + (lcrc >> 16);

	*cval = lcrc;
	*clen = total;
	return (0);
}