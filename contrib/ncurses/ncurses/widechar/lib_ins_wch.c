
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
 *  Author: Thomas Dickey 2002                                              *
 ****************************************************************************/

/*
**	lib_ins_wch.c
**
**	The routine wins_wch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_ins_wch.c,v 1.8 2005/12/03 20:24:19 tom Exp $")

/*
 * Insert the given character, updating the current location to simplify
 * inserting a string.
 */
static int
_nc_insert_wch(WINDOW *win, const cchar_t *wch)
{
    int cells = wcwidth(CharOf(CHDEREF(wch)));
    int cell;

    if (cells <= 0)
	cells = 1;

    if (win->_curx <= win->_maxx) {
	struct ldat *line = &(win->_line[win->_cury]);
	NCURSES_CH_T *end = &(line->text[win->_curx]);
	NCURSES_CH_T *temp1 = &(line->text[win->_maxx]);
	NCURSES_CH_T *temp2 = temp1 - cells;

	CHANGED_TO_EOL(line, win->_curx, win->_maxx);
	while (temp1 > end)
	    *temp1-- = *temp2--;

	*temp1 = _nc_render(win, *wch);
	for (cell = 1; cell < cells; ++cell) {
	    SetWidecExt(temp1[cell], cell);
	}

	win->_curx++;
    }
    return OK;
}

NCURSES_EXPORT(int)
wins_wch(WINDOW *win, const cchar_t *wch)
{
    NCURSES_SIZE_T oy;
    NCURSES_SIZE_T ox;
    int code = ERR;

    T((T_CALLED("wins_wch(%p, %s)"), win, _tracecchar_t(wch)));

    if (win != 0) {
	oy = win->_cury;
	ox = win->_curx;

	code = _nc_insert_wch(win, wch);

	win->_curx = ox;
	win->_cury = oy;
	_nc_synchook(win);
    }
    returnCode(code);
}

NCURSES_EXPORT(int)
wins_nwstr(WINDOW *win, const wchar_t *wstr, int n)
{
    int code = ERR;
    NCURSES_SIZE_T oy;
    NCURSES_SIZE_T ox;
    const wchar_t *cp;

    T((T_CALLED("wins_nwstr(%p,%s,%d)"), win, _nc_viswbufn(wstr, n), n));

    if (win != 0
	&& wstr != 0) {
	if (n < 1)
	    n = wcslen(wstr);
	code = OK;
	if (n > 0) {
	    oy = win->_cury;
	    ox = win->_curx;
	    for (cp = wstr; *cp && ((cp - wstr) < n); cp++) {
		int len = wcwidth(*cp);

		if (len != 1 || !is8bits(*cp)) {
		    cchar_t tmp_cchar;
		    wchar_t tmp_wchar = *cp;
		    memset(&tmp_cchar, 0, sizeof(tmp_cchar));
		    (void) setcchar(&tmp_cchar,
				    &tmp_wchar,
				    WA_NORMAL,
				    0,
				    (void *) 0);
		    code = _nc_insert_wch(win, &tmp_cchar);
		} else {
		    /* tabs, other ASCII stuff */
		    code = _nc_insert_ch(win, (chtype) (*cp));
		}
		if (code != OK)
		    break;
	    }

	    win->_curx = ox;
	    win->_cury = oy;
	    _nc_synchook(win);
	}
    }
    returnCode(code);
}