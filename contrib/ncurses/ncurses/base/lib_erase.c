
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
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
**	lib_erase.c
**
**	The routine werase().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_erase.c,v 1.16 2005/10/30 00:36:36 tom Exp $")

NCURSES_EXPORT(int)
werase(WINDOW *win)
{
    int code = ERR;
    int y;
    NCURSES_CH_T blank;
    NCURSES_CH_T *sp, *end, *start;

    T((T_CALLED("werase(%p)"), win));

    if (win) {
	blank = win->_nc_bkgd;
	for (y = 0; y <= win->_maxy; y++) {
	    start = win->_line[y].text;
	    end = &start[win->_maxx];

	    /*
	     * If this is a derived window, we have to handle the case where
	     * a multicolumn character extends into the window that we are
	     * erasing.
	     */
	    if_WIDEC({
		if (isWidecExt(start[0])) {
		    int x = (win->_parent != 0) ? (win->_begx) : 0;
		    while (x-- > 0) {
			if (isWidecBase(start[-1])) {
			    --start;
			    break;
			}
			--start;
		    }
		}
	    });

	    for (sp = start; sp <= end; sp++)
		*sp = blank;

	    win->_line[y].firstchar = 0;
	    win->_line[y].lastchar = win->_maxx;
	}
	win->_curx = win->_cury = 0;
	win->_flags &= ~_WRAPPED;
	_nc_synchook(win);
	code = OK;
    }
    returnCode(code);
}