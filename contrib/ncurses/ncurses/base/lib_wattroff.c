
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
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
**	lib_wattroff.c
**
**	The routine wattr_off().
**
*/

#include <curses.priv.h>
#include <ctype.h>

MODULE_ID("$Id: lib_wattroff.c,v 1.9 2006/05/27 19:30:33 tom Exp $")

NCURSES_EXPORT(int)
wattr_off(WINDOW *win, attr_t at, void *opts GCC_UNUSED)
{
    T((T_CALLED("wattr_off(%p,%s)"), win, _traceattr(at)));
    if (win) {
	T(("... current %s (%d)",
	   _traceattr(WINDOW_ATTRS(win)),
	   GET_WINDOW_PAIR(win)));

	if_EXT_COLORS({
	    if (at & A_COLOR)
		win->_color = 0;
	});
	toggle_attr_off(WINDOW_ATTRS(win), at);
	returnCode(OK);
    } else
	returnCode(ERR);
}