
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
**	lib_delwin.c
**
**	The routine delwin().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_delwin.c,v 1.17 2008/06/07 14:10:56 tom Exp $")

static bool
cannot_delete(WINDOW *win)
{
    WINDOWLIST *p;
    bool result = TRUE;

    for (each_window(p)) {
	if (&(p->win) == win) {
	    result = FALSE;
	} else if ((p->win._flags & _SUBWIN) != 0
		   && p->win._parent == win) {
	    result = TRUE;
	    break;
	}
    }
    return result;
}

NCURSES_EXPORT(int)
delwin(WINDOW *win)
{
    int result = ERR;

    T((T_CALLED("delwin(%p)"), win));

    if (_nc_try_global(curses) == 0) {
	if (win == 0
	    || cannot_delete(win)) {
	    result = ERR;
	} else {

	    if (win->_flags & _SUBWIN)
		touchwin(win->_parent);
	    else if (curscr != 0)
		touchwin(curscr);

	    result = _nc_freewin(win);
	}
	_nc_unlock_global(curses);
    }
    returnCode(result);
}