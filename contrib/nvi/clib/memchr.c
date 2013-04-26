
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

#include "config.h"

#if defined(LIBC_SCCS) && !defined(lint)
static const char sccsid[] = "@(#)memchr.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <string.h>

/*
 * PUBLIC: #ifndef HAVE_MEMCHR
 * PUBLIC: void *memchr __P((const void *, int, size_t));
 * PUBLIC: #endif
 */
void *
memchr(s, c, n)
	const void *s;
	register unsigned char c;
	register size_t n;
{
	if (n != 0) {
		register const unsigned char *p = s;

		do {
			if (*p++ == c)
				return ((void *)(p - 1));
		} while (--n != 0);
	}
	return (NULL);
}