
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
#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strncmp.c	8.1 (Berkeley) 6/4/93";
__RCSID("$NetBSD: wcsncmp.c,v 1.3 2001/01/05 12:13:13 itojun Exp $");
#endif /* LIBC_SCCS and not lint */
#endif
__FBSDID("$FreeBSD$");

#include <wchar.h>

int
wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{

	if (n == 0)
		return (0);
	do {
		if (*s1 != *s2++) {
			/* XXX assumes wchar_t = int */
			return (*(const unsigned int *)s1 -
			    *(const unsigned int *)--s2);
		}
		if (*s1++ == 0)
			break;
	} while (--n != 0);
	return (0);
}