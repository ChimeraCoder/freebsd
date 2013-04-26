
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
**	lib_vline_set.c
**
**	The routine wvline_set().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_vline_set.c,v 1.2 2002/03/23 21:36:01 tom Exp $")

NCURSES_EXPORT(int)
wvline_set(WINDOW *win, const cchar_t * ch, int n)
{
    int code = ERR;
    NCURSES_SIZE_T row, col;
    NCURSES_SIZE_T end;

    T((T_CALLED("wvline(%p,%s,%d)"), win, _tracecchar_t(ch), n));

    if (win) {
	NCURSES_CH_T wch;
	row = win->_cury;
	col = win->_curx;
	end = row + n - 1;
	if (end > win->_maxy)
	    end = win->_maxy;

	if (ch == 0)
	    wch = *WACS_VLINE;
	else
	    wch = *ch;
	wch = _nc_render(win, wch);

	while (end >= row) {
	    struct ldat *line = &(win->_line[end]);
	    line->text[col] = wch;
	    CHANGED_CELL(line, col);
	    end--;
	}

	_nc_synchook(win);
	code = OK;
    }
    returnCode(code);
}