
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
**	lib_in_wchnstr.c
**
**	The routine win_wchnstr().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_in_wchnstr.c,v 1.7 2007/02/11 01:00:00 tom Exp $")

NCURSES_EXPORT(int)
win_wchnstr(WINDOW *win, cchar_t *wchstr, int n)
{
    int code = OK;

    T((T_CALLED("win_wchnstr(%p,%p,%d)"), win, wchstr, n));
    if (win != 0
	&& wchstr != 0) {
	NCURSES_CH_T *src;
	int row, col;
	int j, k, limit;

	getyx(win, row, col);
	limit = getmaxx(win) - col;
	src = &(win->_line[row].text[col]);

	if (n < 0) {
	    n = limit;
	} else if (n > limit) {
	    n = limit;
	}
	for (j = k = 0; j < n; ++j) {
	    if (j == 0 || !WidecExt(src[j]) || isWidecBase(src[j])) {
		wchstr[k++] = src[j];
	    }
	}
	memset(&(wchstr[k]), 0, sizeof(*wchstr));
	T(("result = %s", _nc_viscbuf(wchstr, n)));
    } else {
	code = ERR;
    }
    returnCode(code);
}