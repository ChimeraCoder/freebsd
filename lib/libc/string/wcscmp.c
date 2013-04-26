
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
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strcmp.c	8.1 (Berkeley) 6/4/93";
#if 0
__RCSID("$NetBSD: wcscmp.c,v 1.3 2001/01/05 12:13:12 itojun Exp $");
#endif
#endif /* LIBC_SCCS and not lint */
__FBSDID("$FreeBSD$");

#include <wchar.h>

/*
 * Compare strings.
 */
int
wcscmp(const wchar_t *s1, const wchar_t *s2)
{

	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	/* XXX assumes wchar_t = int */
	return (*(const unsigned int *)s1 - *(const unsigned int *)--s2);
}