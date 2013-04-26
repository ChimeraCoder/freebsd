
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
 *     Author: Thomas E. Dickey                        2007                 *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: use_window.c,v 1.8 2008/06/07 14:13:46 tom Exp $")

NCURSES_EXPORT(int)
use_window(WINDOW *win, NCURSES_WINDOW_CB func, void *data)
{
    int code = OK;

    T((T_CALLED("use_window(%p,%p,%p)"), win, func, data));
    _nc_lock_global(curses);
    code = func(win, data);
    _nc_unlock_global(curses);

    returnCode(code);
}