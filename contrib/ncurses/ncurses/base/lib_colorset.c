
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
 *  Author: Juergen Pfeifer,  1998                                          *
 *     and: Thomas E. Dickey, 2005                                          *
 ****************************************************************************/

/*
**	lib_colorset.c
**
**	The routine wcolor_set().
**
*/

#include <curses.priv.h>
#include <ctype.h>

MODULE_ID("$Id: lib_colorset.c,v 1.11 2005/01/29 21:40:51 tom Exp $")

NCURSES_EXPORT(int)
wcolor_set(WINDOW *win, short color_pair_number, void *opts)
{
    T((T_CALLED("wcolor_set(%p,%d)"), win, color_pair_number));
    if (win
	&& !opts
	&& (color_pair_number >= 0)
	&& (color_pair_number < COLOR_PAIRS)) {
	TR(TRACE_ATTRS, ("... current %ld", (long) GET_WINDOW_PAIR(win)));
	SET_WINDOW_PAIR(win, color_pair_number);
	if_EXT_COLORS(win->_color = color_pair_number);
	returnCode(OK);
    } else
	returnCode(ERR);
}