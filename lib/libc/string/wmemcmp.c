
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
__RCSID("$NetBSD: wmemcmp.c,v 1.1 2000/12/23 23:14:37 itojun Exp $");
#endif /* LIBC_SCCS and not lint */
#endif
__FBSDID("$FreeBSD$");

#include <wchar.h>

int
wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++) {
		if (*s1 != *s2) {
			/* wchar might be unsigned */
			return *s1 > *s2 ? 1 : -1; 
		}
		s1++;
		s2++;
	}
	return 0;
}