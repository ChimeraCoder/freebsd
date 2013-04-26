
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

MODULE_ID("$Id: use_screen.c,v 1.6 2008/06/07 19:16:56 tom Exp $")

NCURSES_EXPORT(int)
use_screen(SCREEN *screen, NCURSES_SCREEN_CB func, void *data)
{
    SCREEN *save_SP;
    int code = OK;

    T((T_CALLED("use_screen(%p,%p,%p)"), screen, func, data));

    /*
     * FIXME - add a flag so a given thread can check if _it_ has already
     * recurred through this point, return an error if so.
     */
    _nc_lock_global(curses);
    save_SP = SP;
    set_term(screen);

    code = func(screen, data);

    set_term(save_SP);
    _nc_unlock_global(curses);
    returnCode(code);
}