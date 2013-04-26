
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
#include <string.h>
#include <wctype.h>
#include "xlocale_private.h"

enum {
	_WCT_ERROR	= 0,
	_WCT_TOLOWER	= 1,
	_WCT_TOUPPER	= 2
};

wint_t
towctrans_l(wint_t wc, wctrans_t desc, locale_t locale)
{
	switch (desc) {
	case _WCT_TOLOWER:
		wc = towlower_l(wc, locale);
		break;
	case _WCT_TOUPPER:
		wc = towupper_l(wc, locale);
		break;
	case _WCT_ERROR:
	default:
		errno = EINVAL;
		break;
	}

	return (wc);
}
wint_t
towctrans(wint_t wc, wctrans_t desc)
{
	return towctrans_l(wc, desc, __get_locale());
}

/*
 * wctrans() calls this will a 0 locale.  If this is ever modified to actually
 * use the locale, wctrans() must be modified to call __get_locale().
 */
wctrans_t
wctrans_l(const char *charclass, locale_t locale)
{
	struct {
		const char	*name;
		wctrans_t	 trans;
	} ccls[] = {
		{ "tolower",	_WCT_TOLOWER },
		{ "toupper",	_WCT_TOUPPER },
		{ NULL,		_WCT_ERROR },		/* Default */
	};
	int i;

	i = 0;
	while (ccls[i].name != NULL && strcmp(ccls[i].name, charclass) != 0)
		i++;

	if (ccls[i].trans == _WCT_ERROR)
		errno = EINVAL;
	return (ccls[i].trans);
}

wctrans_t
wctrans(const char *charclass)
{
	return wctrans_l(charclass, 0);
}