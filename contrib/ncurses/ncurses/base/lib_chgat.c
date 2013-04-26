
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
 *     and: Sven Verdoolaege                        2001                    *
 *     and: Thomas E. Dickey                        2005                    *
 ****************************************************************************/

/*
**	lib_chgat.c
**
**	The routine wchgat().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_chgat.c,v 1.7 2006/07/15 22:07:11 tom Exp $")

NCURSES_EXPORT(int)
wchgat(WINDOW *win, int n, attr_t attr, short color, const void *opts GCC_UNUSED)
{
    int i;

    T((T_CALLED("wchgat(%p,%d,%s,%d)"), win, n, _traceattr(attr), color));

    if (win) {
	struct ldat *line = &(win->_line[win->_cury]);

	toggle_attr_on(attr, COLOR_PAIR(color));

	for (i = win->_curx; i <= win->_maxx && (n == -1 || (n-- > 0)); i++) {
	    SetAttr(line->text[i], attr);
	    SetPair(line->text[i], color);
	    CHANGED_CELL(line, i);
	}

	returnCode(OK);
    } else
	returnCode(ERR);
}