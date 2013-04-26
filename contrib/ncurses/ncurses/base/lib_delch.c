
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
**	lib_delch.c
**
**	The routine wdelch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_delch.c,v 1.12 2001/12/19 01:06:09 tom Exp $")

NCURSES_EXPORT(int)
wdelch(WINDOW *win)
{
    int code = ERR;

    T((T_CALLED("wdelch(%p)"), win));

    if (win) {
	NCURSES_CH_T blank = win->_nc_bkgd;
	struct ldat *line = &(win->_line[win->_cury]);
	NCURSES_CH_T *end = &(line->text[win->_maxx]);
	NCURSES_CH_T *temp2 = &(line->text[win->_curx + 1]);
	NCURSES_CH_T *temp1 = temp2 - 1;

	CHANGED_TO_EOL(line, win->_curx, win->_maxx);
	while (temp1 < end)
	    *temp1++ = *temp2++;

	*temp1 = blank;

	_nc_synchook(win);
	code = OK;
    }
    returnCode(code);
}