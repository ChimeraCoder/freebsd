
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

#include <ctype.h>
#include <string.h>
#include "xlocale_private.h"

/*
 * Find the first occurrence of find in s, ignore case.
 */
char *
strcasestr_l(const char *s, const char *find, locale_t locale)
{
	char c, sc;
	size_t len;
	FIX_LOCALE(locale);

	if ((c = *find++) != 0) {
		c = tolower_l((unsigned char)c, locale);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower_l((unsigned char)sc, locale) != c);
		} while (strncasecmp_l(s, find, len, locale) != 0);
		s--;
	}
	return ((char *)s);
}
char *
strcasestr(const char *s, const char *find)
{
	return strcasestr_l(s, find, __get_locale());
}