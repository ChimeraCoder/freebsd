
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

/*
**	lib_cchar.c
**
**	The routines setcchar() and getcchar().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_cchar.c,v 1.12 2007/05/12 19:03:06 tom Exp $")

/* 
 * The SuSv2 description leaves some room for interpretation.  We'll assume wch
 * points to a string which is L'\0' terminated, contains at least one
 * character with strictly positive width, which must be the first, and
 * contains no characters of negative width.
 */
NCURSES_EXPORT(int)
setcchar(cchar_t *wcval,
	 const wchar_t *wch,
	 const attr_t attrs,
	 short color_pair,
	 const void *opts)
{
    int i;
    int len;
    int code = OK;

    TR(TRACE_CCALLS, (T_CALLED("setcchar(%p,%s,%lu,%d,%p)"),
		      wcval, _nc_viswbuf(wch),
		      (unsigned long) attrs, color_pair, opts));

    len = wcslen(wch);
    if (opts != NULL
	|| (len > 1 && wcwidth(wch[0]) < 0)) {
	code = ERR;
    } else {
	if (len > CCHARW_MAX)
	    len = CCHARW_MAX;

	/*
	 * If we have a following spacing-character, stop at that point.  We
	 * are only interested in adding non-spacing characters.
	 */
	for (i = 1; i < len; ++i) {
	    if (wcwidth(wch[i]) != 0) {
		len = i;
		break;
	    }
	}

	memset(wcval, 0, sizeof(*wcval));

	if (len != 0) {
	    SetAttr(*wcval, attrs | COLOR_PAIR(color_pair));
	    SetPair(CHDEREF(wcval), color_pair);
	    memcpy(&wcval->chars, wch, len * sizeof(wchar_t));
	    TR(TRACE_CCALLS, ("copy %d wchars, first is %s", len,
			      _tracecchar_t(wcval)));
	}
    }

    TR(TRACE_CCALLS, (T_RETURN("%d"), code));
    return (code);
}

NCURSES_EXPORT(int)
getcchar(const cchar_t *wcval,
	 wchar_t *wch,
	 attr_t *attrs,
	 short *color_pair,
	 void *opts)
{
    wchar_t *wp;
    int len;
    int code = ERR;

    TR(TRACE_CCALLS, (T_CALLED("getcchar(%p,%p,%p,%p,%p)"),
		      wcval, wch, attrs, color_pair, opts));

    if (opts == NULL) {
	len = (wp = wmemchr(wcval->chars, L'\0', CCHARW_MAX))
	    ? wp - wcval->chars
	    : CCHARW_MAX;

	if (wch == NULL) {
	    code = len;
	} else if (attrs == 0 || color_pair == 0) {
	    code = ERR;
	} else if (len >= 0) {
	    *attrs = AttrOf(*wcval) & A_ATTRIBUTES;
	    *color_pair = GetPair(*wcval);
	    wmemcpy(wch, wcval->chars, (unsigned) len);
	    wch[len] = L'\0';
	    code = OK;
	}
    }

    TR(TRACE_CCALLS, (T_RETURN("%d"), code));
    return (code);
}