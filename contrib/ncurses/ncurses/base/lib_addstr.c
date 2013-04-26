
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

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *                                                                          *
 *  Rewritten 2001-2004 to support wide-characters by                       *
 *	Sven Verdoolaege                                                    *
 *	Thomas Dickey                                                       *
 ****************************************************************************/

/*
**	lib_addstr.c
*
**	The routines waddnstr(), waddchnstr().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_addstr.c,v 1.48 2007/10/13 19:56:57 tom Exp $")

NCURSES_EXPORT(int)
waddnstr(WINDOW *win, const char *astr, int n)
{
    const char *str = astr;
    int code = ERR;

    T((T_CALLED("waddnstr(%p,%s,%d)"), win, _nc_visbufn(astr, n), n));

    if (win && (str != 0)) {
	TR(TRACE_VIRTPUT | TRACE_ATTRS,
	   ("... current %s", _traceattr(WINDOW_ATTRS(win))));
	code = OK;
	if (n < 0)
	    n = (int) strlen(astr);

	TR(TRACE_VIRTPUT, ("str is not null, length = %d", n));
	while ((n-- > 0) && (*str != '\0')) {
	    NCURSES_CH_T ch;
	    TR(TRACE_VIRTPUT, ("*str = %#o", UChar(*str)));
	    SetChar(ch, UChar(*str++), A_NORMAL);
	    if (_nc_waddch_nosync(win, ch) == ERR) {
		code = ERR;
		break;
	    }
	}
	_nc_synchook(win);
    }
    TR(TRACE_VIRTPUT, ("waddnstr returns %d", code));
    returnCode(code);
}

NCURSES_EXPORT(int)
waddchnstr(WINDOW *win, const chtype *astr, int n)
{
    NCURSES_SIZE_T y, x;
    int code = OK;
    int i;
    struct ldat *line;

    T((T_CALLED("waddchnstr(%p,%p,%d)"), win, astr, n));

    if (!win)
	returnCode(ERR);

    y = win->_cury;
    x = win->_curx;
    if (n < 0) {
	const chtype *str;
	n = 0;
	for (str = (const chtype *) astr; *str != 0; str++)
	    n++;
    }
    if (n > win->_maxx - x + 1)
	n = win->_maxx - x + 1;
    if (n == 0)
	returnCode(code);

    line = &(win->_line[y]);
    for (i = 0; i < n && ChCharOf(astr[i]) != '\0'; ++i) {
	SetChar2(line->text[i + x], astr[i]);
    }
    CHANGED_RANGE(line, x, x + n - 1);

    _nc_synchook(win);
    returnCode(code);
}

#if USE_WIDEC_SUPPORT

NCURSES_EXPORT(int)
_nc_wchstrlen(const cchar_t *s)
{
    int result = 0;
    while (CharOf(s[result]) != L'\0') {
	result++;
    }
    return result;
}

NCURSES_EXPORT(int)
wadd_wchnstr(WINDOW *win, const cchar_t *astr, int n)
{
    static const NCURSES_CH_T blank = NewChar(BLANK_TEXT);
    NCURSES_SIZE_T y;
    NCURSES_SIZE_T x;
    int code = OK;
    struct ldat *line;
    int i, j, start, len, end;

    T((T_CALLED("wadd_wchnstr(%p,%s,%d)"), win, _nc_viscbuf(astr, n), n));

    if (!win)
	returnCode(ERR);

    y = win->_cury;
    x = win->_curx;
    if (n < 0) {
	n = _nc_wchstrlen(astr);
    }
    if (n > win->_maxx - x + 1)
	n = win->_maxx - x + 1;
    if (n == 0)
	returnCode(code);

    line = &(win->_line[y]);
    start = x;
    end = x + n - 1;

    /*
     * Reset orphaned cells of multi-column characters that extend up to the
     * new string's location to blanks.
     */
    if (x > 0 && isWidecExt(line->text[x])) {
	for (i = 0; i <= x; ++i) {
	    if (!isWidecExt(line->text[x - i])) {
		/* must be isWidecBase() */
		start -= i;
		while (i > 0) {
		    line->text[x - i--] = _nc_render(win, blank);
		}
		break;
	    }
	}
    }

    /*
     * Copy the new string to the window.
     */
    for (i = 0; i < n && CharOf(astr[i]) != L'\0' && x <= win->_maxx; ++i) {
	if (isWidecExt(astr[i]))
	    continue;

	len = wcwidth(CharOf(astr[i]));

	if (x + len - 1 <= win->_maxx) {
	    line->text[x] = _nc_render(win, astr[i]);
	    if (len > 1) {
		for (j = 0; j < len; ++j) {
		    if (j != 0) {
			line->text[x + j] = line->text[x];
		    }
		    SetWidecExt(line->text[x + j], j);
		}
	    }
	    x += len;
	    end += len - 1;
	} else {
	    break;
	}
    }

    /*
     * Set orphaned cells of multi-column characters which lie after the new
     * string to blanks.
     */
    while (x <= win->_maxx && isWidecExt(line->text[x])) {
	line->text[x] = _nc_render(win, blank);
	++end;
	++x;
    }
    CHANGED_RANGE(line, start, end);

    _nc_synchook(win);
    returnCode(code);
}

NCURSES_EXPORT(int)
waddnwstr(WINDOW *win, const wchar_t *str, int n)
{
    int code = ERR;

    T((T_CALLED("waddnwstr(%p,%s,%d)"), win, _nc_viswbufn(str, n), n));

    if (win && (str != 0)) {
	TR(TRACE_VIRTPUT | TRACE_ATTRS,
	   ("... current %s", _traceattr(WINDOW_ATTRS(win))));
	code = OK;
	if (n < 0)
	    n = (int) wcslen(str);

	TR(TRACE_VIRTPUT, ("str is not null, length = %d", n));
	while ((n-- > 0) && (*str != L('\0'))) {
	    NCURSES_CH_T ch;
	    TR(TRACE_VIRTPUT, ("*str[0] = %#lx", (unsigned long) *str));
	    SetChar(ch, *str++, A_NORMAL);
	    if (wadd_wch(win, &ch) == ERR) {
		code = ERR;
		break;
	    }
	}
	_nc_synchook(win);
    }
    TR(TRACE_VIRTPUT, ("waddnwstr returns %d", code));
    returnCode(code);
}

#endif