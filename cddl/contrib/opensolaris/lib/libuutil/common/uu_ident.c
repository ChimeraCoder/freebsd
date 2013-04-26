
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include "libuutil_common.h"

#include <string.h>

/*
 * We require names of the form:
 *	[provider,]identifier[/[provider,]identifier]...
 *
 * Where provider is either a stock symbol (SUNW) or a java-style reversed
 * domain name (com.sun).
 *
 * Both providers and identifiers must start with a letter, and may
 * only contain alphanumerics, dashes, and underlines.  Providers
 * may also contain periods.
 *
 * Note that we do _not_ use the macros in <ctype.h>, since they are affected
 * by the current locale settings.
 */

#define	IS_ALPHA(c) \
	(((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))

#define	IS_DIGIT(c) \
	((c) >= '0' && (c) <= '9')

static int
is_valid_ident(const char *s, const char *e, int allowdot)
{
	char c;

	if (s >= e)
		return (0);		/* name is empty */

	c = *s++;
	if (!IS_ALPHA(c))
		return (0);		/* does not start with letter */

	while (s < e && (c = *s++) != 0) {
		if (IS_ALPHA(c) || IS_DIGIT(c) || c == '-' || c == '_' ||
		    (allowdot && c == '.'))
			continue;
		return (0);		/* invalid character */
	}
	return (1);
}

static int
is_valid_component(const char *b, const char *e, uint_t flags)
{
	char *sp;

	if (flags & UU_NAME_DOMAIN) {
		sp = strchr(b, ',');
		if (sp != NULL && sp < e) {
			if (!is_valid_ident(b, sp, 1))
				return (0);
			b = sp + 1;
		}
	}

	return (is_valid_ident(b, e, 0));
}

int
uu_check_name(const char *name, uint_t flags)
{
	const char *end = name + strlen(name);
	const char *p;

	if (flags & ~(UU_NAME_DOMAIN | UU_NAME_PATH)) {
		uu_set_error(UU_ERROR_UNKNOWN_FLAG);
		return (-1);
	}

	if (!(flags & UU_NAME_PATH)) {
		if (!is_valid_component(name, end, flags))
			goto bad;
		return (0);
	}

	while ((p = strchr(name, '/')) != NULL) {
		if (!is_valid_component(name, p - 1, flags))
			goto bad;
		name = p + 1;
	}
	if (!is_valid_component(name, end, flags))
		goto bad;

	return (0);

bad:
	uu_set_error(UU_ERROR_INVALID_ARGUMENT);
	return (-1);
}