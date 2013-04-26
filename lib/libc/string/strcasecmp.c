
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strcasecmp.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <strings.h>
#include <ctype.h>
#include "xlocale_private.h"

int
strcasecmp_l(const char *s1, const char *s2, locale_t locale)
{
	const u_char
			*us1 = (const u_char *)s1,
			*us2 = (const u_char *)s2;
	FIX_LOCALE(locale);

	while (tolower_l(*us1, locale) == tolower_l(*us2++, locale))
		if (*us1++ == '\0')
			return (0);
	return (tolower_l(*us1, locale) - tolower_l(*--us2, locale));
}
int
strcasecmp(const char *s1, const char *s2)
{
	return strcasecmp_l(s1, s2, __get_locale());
}

int
strncasecmp_l(const char *s1, const char *s2, size_t n, locale_t locale)
{
	FIX_LOCALE(locale);
	if (n != 0) {
		const u_char
				*us1 = (const u_char *)s1,
				*us2 = (const u_char *)s2;

		do {
			if (tolower_l(*us1, locale) != tolower_l(*us2++, locale))
				return (tolower_l(*us1, locale) - tolower_l(*--us2, locale));
			if (*us1++ == '\0')
				break;
		} while (--n != 0);
	}
	return (0);
}

int
strncasecmp(const char *s1, const char *s2, size_t n)
{
	return strncasecmp_l(s1, s2, n, __get_locale());
}