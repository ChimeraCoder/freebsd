
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
**	lib_clear.c
**
**	The routine wclear().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_clear.c,v 1.7 2000/12/10 02:43:26 tom Exp $")

NCURSES_EXPORT(int)
wclear(WINDOW *win)
{
    int code = ERR;

    T((T_CALLED("wclear(%p)"), win));

    if ((code = werase(win)) != ERR)
	win->_clear = TRUE;

    returnCode(code);
}