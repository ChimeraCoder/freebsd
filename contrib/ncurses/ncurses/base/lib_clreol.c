
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
**	lib_clreol.c
**
**	The routine wclrtoeol().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_clreol.c,v 1.21 2001/12/19 01:06:04 tom Exp $")

NCURSES_EXPORT(int)
wclrtoeol(WINDOW *win)
{
    int code = ERR;

    T((T_CALLED("wclrtoeol(%p)"), win));

    if (win) {
	NCURSES_CH_T blank;
	NCURSES_CH_T *ptr, *end;
	struct ldat *line;
	NCURSES_SIZE_T y = win->_cury;
	NCURSES_SIZE_T x = win->_curx;

	/*
	 * If we have just wrapped the cursor, the clear applies to the
	 * new line, unless we are at the lower right corner.
	 */
	if ((win->_flags & _WRAPPED) != 0
	    && y < win->_maxy) {
	    win->_flags &= ~_WRAPPED;
	}

	/*
	 * There's no point in clearing if we're not on a legal
	 * position, either.
	 */
	if ((win->_flags & _WRAPPED) != 0
	    || y > win->_maxy
	    || x > win->_maxx)
	    returnCode(ERR);

	blank = win->_nc_bkgd;
	line = &win->_line[y];
	CHANGED_TO_EOL(line, x, win->_maxx);

	ptr = &(line->text[x]);
	end = &(line->text[win->_maxx]);

	while (ptr <= end)
	    *ptr++ = blank;

	_nc_synchook(win);
	code = OK;
    }
    returnCode(code);
}