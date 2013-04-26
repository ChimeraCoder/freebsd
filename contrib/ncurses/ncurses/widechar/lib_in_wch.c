
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
**	lib_in_wch.c
**
**	The routine win_wch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_in_wch.c,v 1.4 2006/09/03 15:41:22 tom Exp $")

NCURSES_EXPORT(int)
win_wch(WINDOW *win, cchar_t *wcval)
{
    int row, col;
    int code = OK;

    TR(TRACE_CCALLS, (T_CALLED("win_wch(%p,%p)"), win, wcval));
    if (win != 0
	&& wcval != 0) {
	getyx(win, row, col);

	*wcval = win->_line[row].text[col];
	TR(TRACE_CCALLS, ("data %s", _tracecchar_t(wcval)));
    } else {
	code = ERR;
    }
    TR(TRACE_CCALLS, (T_RETURN("%d"), code));
    return (code);
}