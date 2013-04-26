
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
static char sccsid[] = "@(#)print.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <stdio.h>
#include <stdint.h>

#include "extern.h"

void
pcrc(char *fn, uint32_t val, off_t len)
{
	(void)printf("%lu %jd", (u_long)val, (intmax_t)len);
	if (fn != NULL)
		(void)printf(" %s", fn);
	(void)printf("\n");
}

void
psum1(char *fn, uint32_t val, off_t len)
{
	(void)printf("%lu %jd", (u_long)val, (intmax_t)(len + 1023) / 1024);
	if (fn != NULL)
		(void)printf(" %s", fn);
	(void)printf("\n");
}

void
psum2(char *fn, uint32_t val, off_t len)
{
	(void)printf("%lu %jd", (u_long)val, (intmax_t)(len + 511) / 512);
	if (fn != NULL)
		(void)printf(" %s", fn);
	(void)printf("\n");
}