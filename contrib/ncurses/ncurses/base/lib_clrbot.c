
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
**	lib_clrbot.c
**
**	The routine wclrtobot().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_clrbot.c,v 1.20 2006/10/14 20:43:31 tom Exp $")

NCURSES_EXPORT(int)
wclrtobot(WINDOW *win)
{
    int code = ERR;

    T((T_CALLED("wclrtobot(%p)"), win));

    if (win) {
	NCURSES_SIZE_T y;
	NCURSES_SIZE_T startx = win->_curx;
	NCURSES_CH_T blank = win->_nc_bkgd;

	T(("clearing from y = %ld to y = %ld with maxx =  %ld",
	   (long) win->_cury, (long) win->_maxy, (long) win->_maxx));

	for (y = win->_cury; y <= win->_maxy; y++) {
	    struct ldat *line = &(win->_line[y]);
	    NCURSES_CH_T *ptr = &(line->text[startx]);
	    NCURSES_CH_T *end = &(line->text[win->_maxx]);

	    CHANGED_TO_EOL(line, startx, win->_maxx);

	    while (ptr <= end)
		*ptr++ = blank;

	    startx = 0;
	}
	_nc_synchook(win);
	code = OK;
    }
    returnCode(code);
}