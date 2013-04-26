
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1998                        *
 ****************************************************************************/

/*
**	lib_winch.c
**
**	The routine winch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_winch.c,v 1.5 2001/06/02 23:42:08 skimo Exp $")

NCURSES_EXPORT(chtype)
winch(WINDOW *win)
{
    T((T_CALLED("winch(%p)"), win));
    if (win != 0) {
	returnChar(CharOf(win->_line[win->_cury].text[win->_curx]) |
		   AttrOf(win->_line[win->_cury].text[win->_curx]));
    } else {
	returnChar(0);
    }
}