
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
**	lib_hline.c
**
**	The routine whline().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_hline.c,v 1.11 2006/03/11 21:52:27 tom Exp $")

NCURSES_EXPORT(int)
whline(WINDOW *win, chtype ch, int n)
{
    int code = ERR;
    NCURSES_SIZE_T start;
    NCURSES_SIZE_T end;

    T((T_CALLED("whline(%p,%s,%d)"), win, _tracechtype(ch), n));

    if (win) {
	struct ldat *line = &(win->_line[win->_cury]);
	NCURSES_CH_T wch;

	start = win->_curx;
	end = start + n - 1;
	if (end > win->_maxx)
	    end = win->_maxx;

	CHANGED_RANGE(line, start, end);

	if (ch == 0)
	    SetChar2(wch, ACS_HLINE);
	else
	    SetChar2(wch, ch);
	wch = _nc_render(win, wch);

	while (end >= start) {
	    line->text[end] = wch;
	    end--;
	}

	_nc_synchook(win);
	code = OK;
    }
    returnCode(code);
}