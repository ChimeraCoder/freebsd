
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
**	lib_insdel.c
**
**	The routine winsdelln(win, n).
**  positive n insert n lines above current line
**  negative n delete n lines starting from current line
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_insdel.c,v 1.12 2003/07/26 22:40:06 tom Exp $")

NCURSES_EXPORT(int)
winsdelln(WINDOW *win, int n)
{
    int code = ERR;

    T((T_CALLED("winsdelln(%p,%d)"), win, n));

    if (win) {
	if (n != 0) {
	    _nc_scroll_window(win, -n, win->_cury, win->_maxy,
			      win->_nc_bkgd);
	    _nc_synchook(win);
	}
	code = OK;
    }
    returnCode(code);
}