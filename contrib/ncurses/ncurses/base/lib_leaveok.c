
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
**	lib_leaveok.c
**
**	The routine leaveok.
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_leaveok.c,v 1.5 2000/12/10 02:43:27 tom Exp $")

NCURSES_EXPORT(int)
leaveok(WINDOW *win, bool flag)
{
    T((T_CALLED("leaveok(%p,%d)"), win, flag));

    if (win) {
	win->_leaveok = flag;
	returnCode(OK);
    } else
	returnCode(ERR);
}