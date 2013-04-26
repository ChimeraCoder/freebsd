
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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

/*
**	lib_insnstr.c
**
**	The routine winsnstr().
**
*/

#include <curses.priv.h>
#include <ctype.h>

MODULE_ID("$Id: lib_insnstr.c,v 1.1 2004/02/28 23:44:56 tom Exp $")

NCURSES_EXPORT(int)
winsnstr(WINDOW *win, const char *s, int n)
{
    int code = ERR;
    NCURSES_SIZE_T oy;
    NCURSES_SIZE_T ox;
    const unsigned char *str = (const unsigned char *) s;
    const unsigned char *cp;

    T((T_CALLED("winsnstr(%p,%s,%d)"), win, _nc_visbufn(s, n), n));

    if (win != 0 && str != 0) {
	oy = win->_cury;
	ox = win->_curx;
	for (cp = str; *cp && (n <= 0 || (cp - str) < n); cp++) {
	    _nc_insert_ch(win, (chtype) UChar(*cp));
	}
	win->_curx = ox;
	win->_cury = oy;
	_nc_synchook(win);
	code = OK;
    }
    returnCode(code);
}