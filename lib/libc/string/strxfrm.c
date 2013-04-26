
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

#include <stdlib.h>
#include <string.h>
#include "collate.h"

size_t
strxfrm_l(char * __restrict dest, const char * __restrict src, size_t len, locale_t loc);
size_t
strxfrm(char * __restrict dest, const char * __restrict src, size_t len)
{
	return strxfrm_l(dest, src, len, __get_locale());
}

size_t
strxfrm_l(char * __restrict dest, const char * __restrict src, size_t len, locale_t locale)
{
	int prim, sec, l;
	size_t slen;
	char *s, *ss;
	FIX_LOCALE(locale);
	struct xlocale_collate *table =
		(struct xlocale_collate*)locale->components[XLC_COLLATE];

	if (!*src) {
		if (len > 0)
			*dest = '\0';
		return 0;
	}

	if (table->__collate_load_error)
		return strlcpy(dest, src, len);

	slen = 0;
	prim = sec = 0;
	ss = s = __collate_substitute(table, src);
	while (*s) {
		while (*s && !prim) {
			__collate_lookup(table, s, &l, &prim, &sec);
			s += l;
		}
		if (prim) {
			if (len > 1) {
				*dest++ = (char)prim;
				len--;
			}
			slen++;
			prim = 0;
		}
	}
	free(ss);
	if (len > 0)
		*dest = '\0';

	return slen;
}