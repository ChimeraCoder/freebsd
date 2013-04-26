
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

#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strncpy.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <wchar.h>

/*
 * Copy src to dst, truncating or null-padding to always copy n bytes.
 * Return dst.
 */
wchar_t *
wcsncpy(wchar_t * __restrict dst, const wchar_t * __restrict src, size_t n)
{
	if (n != 0) {
		wchar_t *d = dst;
		const wchar_t *s = src;

		do {
			if ((*d++ = *s++) == L'\0') {
				/* NUL pad the remaining n-1 bytes */
				while (--n != 0)
					*d++ = L'\0';
				break;
			}
		} while (--n != 0);
	}
	return (dst);
}