
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
**	lib_move.c
**
**	The routine wmove().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_move.c,v 1.12 2004/12/04 21:50:07 tom Exp $")

NCURSES_EXPORT(int)
wmove(WINDOW *win, int y, int x)
{
    T((T_CALLED("wmove(%p,%d,%d)"), win, y, x));

    if (LEGALYX(win, y, x)) {
	win->_curx = (NCURSES_SIZE_T) x;
	win->_cury = (NCURSES_SIZE_T) y;

	win->_flags &= ~_WRAPPED;
	win->_flags |= _HASMOVED;
	returnCode(OK);
    } else
	returnCode(ERR);
}