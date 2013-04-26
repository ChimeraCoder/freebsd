
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
__FBSDID("$FreeBSD$");

#include <wchar.h>
#include <wctype.h>

int
wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	wchar_t c1, c2;

	if (n == 0)
		return (0);
	for (; *s1; s1++, s2++) {
		c1 = towlower(*s1);
		c2 = towlower(*s2);
		if (c1 != c2)
			return ((int)c1 - c2);
		if (--n == 0)
			return (0);
	}
	return (-*s2);
}