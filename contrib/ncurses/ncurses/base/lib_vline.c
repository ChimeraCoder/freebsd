
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
 ****************************************************************************/

/*
**	lib_vline.c
**
**	The routine wvline().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_vline.c,v 1.10 2006/03/11 21:52:19 tom Exp $")

NCURSES_EXPORT(int)
wvline(WINDOW *win, chtype ch, int n)
{
    int code = ERR;
    NCURSES_SIZE_T row, col;
    NCURSES_SIZE_T end;

    T((T_CALLED("wvline(%p,%s,%d)"), win, _tracechtype(ch), n));

    if (win) {
	NCURSES_CH_T wch;
	row = win->_cury;
	col = win->_curx;
	end = row + n - 1;
	if (end > win->_maxy)
	    end = win->_maxy;

	if (ch == 0)
	    SetChar2(wch, ACS_VLINE);
	else
	    SetChar2(wch, ch);
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