
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
static const char sccsid[] = "@(#)strpbrk.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <string.h>

/*
 * Find the first occurrence in s1 of a character in s2 (excluding NUL).
 *
 * PUBLIC: #ifndef HAVE_STRPBRK
 * PUBLIC: char *strpbrk __P((const char *, const char *));
 * PUBLIC: #endif
 */
char *
strpbrk(s1, s2)
	register const char *s1, *s2;
{
	register const char *scanp;
	register int c, sc;

	while ((c = *s1++) != 0) {
		for (scanp = s2; (sc = *scanp++) != 0;)
			if (sc == c)
				return ((char *)(s1 - 1));
	}
	return (NULL);
}