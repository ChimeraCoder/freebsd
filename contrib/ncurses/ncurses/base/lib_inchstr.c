
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
**	lib_inchstr.c
**
**	The routine winchnstr().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_inchstr.c,v 1.10 2001/06/02 23:37:58 skimo Exp $")

NCURSES_EXPORT(int)
winchnstr(WINDOW *win, chtype * str, int n)
{
    int i = 0;

    T((T_CALLED("winchnstr(%p,%p,%d)"), win, str, n));

    if (!str)
	returnCode(0);

    if (win) {
	for (; (n < 0 || (i < n)) && (win->_curx + i <= win->_maxx); i++)
	    str[i] =
		CharOf(win->_line[win->_cury].text[win->_curx + i]) |
		AttrOf(win->_line[win->_cury].text[win->_curx + i]);
    }
    str[i] = (chtype) 0;

    returnCode(i);
}