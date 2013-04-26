
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "collate.h"

static char *__mbsdup(const wchar_t *);

/*
 * Placeholder implementation of wcscoll(). Attempts to use the single-byte
 * collation ordering where possible, and falls back on wcscmp() in locales
 * with extended character sets.
 */
int
wcscoll_l(const wchar_t *ws1, const wchar_t *ws2, locale_t locale)
{
	char *mbs1, *mbs2;
	int diff, sverrno;
	FIX_LOCALE(locale);
	struct xlocale_collate *table =
		(struct xlocale_collate*)locale->components[XLC_COLLATE];

	if (table->__collate_load_error || MB_CUR_MAX > 1)
		/*
		 * Locale has no special collating order, could not be
		 * loaded, or has an extended character set; do a fast binary
		 * comparison.
		 */
		return (wcscmp(ws1, ws2));

	if ((mbs1 = __mbsdup(ws1)) == NULL || (mbs2 = __mbsdup(ws2)) == NULL) {
		/*
		 * Out of memory or illegal wide chars; fall back to wcscmp()
		 * but leave errno indicating the error. Callers that don't
		 * check for error will get a reasonable but often slightly
		 * incorrect result.
		 */
		sverrno = errno;
		free(mbs1);
		errno = sverrno;
		return (wcscmp(ws1, ws2));
	}

	diff = strcoll_l(mbs1, mbs2, locale);
	sverrno = errno;
	free(mbs1);
	free(mbs2);
	errno = sverrno;

	return (diff);
}

int
wcscoll(const wchar_t *ws1, const wchar_t *ws2)
{
	return wcscoll_l(ws1, ws2, __get_locale());
}

static char *
__mbsdup(const wchar_t *ws)
{
	static const mbstate_t initial;
	mbstate_t st;
	const wchar_t *wcp;
	size_t len;
	char *mbs;

	wcp = ws;
	st = initial;
	if ((len = wcsrtombs(NULL, &wcp, 0, &st)) == (size_t)-1)
		return (NULL);
	if ((mbs = malloc(len + 1)) == NULL)
		return (NULL);
	st = initial;
	wcsrtombs(mbs, &ws, len + 1, &st);

	return (mbs);
}