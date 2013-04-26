
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
 * Author: Thomas Dickey                                                    *
 ****************************************************************************/

/*
**	lib_inwstr.c
**
**	The routines winnwstr() and winwstr().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_inwstr.c,v 1.4 2004/10/23 20:41:28 tom Exp $")

NCURSES_EXPORT(int)
winnwstr(WINDOW *win, wchar_t *wstr, int n)
{
    int row, col, inx;
    int count = 0;
    int last = 0;
    cchar_t *text;
    wchar_t wch;

    T((T_CALLED("winnwstr(%p,%p,%d)"), win, wstr, n));
    if (wstr != 0) {
	if (win) {
	    getyx(win, row, col);

	    text = win->_line[row].text;
	    while (count < n && count != ERR) {
		if (!isWidecExt(text[col])) {
		    for (inx = 0; (inx < CCHARW_MAX)
			 && ((wch = text[col].chars[inx]) != 0);
			 ++inx) {
			if (count + 1 > n) {
			    if ((count = last) == 0) {
				count = ERR;	/* error if we store nothing */
			    }
			    break;
			}
			wstr[count++] = wch;
		    }
		}
		last = count;
		if (++col > win->_maxx) {
		    break;
		}
	    }
	}
	if (count > 0) {
	    wstr[count] = '\0';
	    T(("winnwstr returns %s", _nc_viswbuf(wstr)));
	}
    }
    returnCode(count);
}

/*
 * X/Open says winwstr() returns OK if not ERR.  If that is not a blunder, it
 * must have a null termination on the string (see above).  Unlike winnstr(),
 * it does not define what happens for a negative count with winnwstr().
 */
NCURSES_EXPORT(int)
winwstr(WINDOW *win, wchar_t *wstr)
{
    int result = OK;
    T((T_CALLED("winwstr(%p,%p)"), win, wstr));
    if (winnwstr(win, wstr, CCHARW_MAX * (win->_maxx - win->_curx + 1)) == ERR)
	result = ERR;
    returnCode(result);
}