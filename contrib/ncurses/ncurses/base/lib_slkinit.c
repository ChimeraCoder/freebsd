
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
 *	lib_slkinit.c
 *	Soft key routines.
 *      Initialize soft labels.  Called by the user before initscr().
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkinit.c,v 1.7 2008/01/12 20:23:39 tom Exp $")

NCURSES_EXPORT(int)
slk_init(int format)
{
    int code = ERR;

    T((T_CALLED("slk_init(%d)"), format));
    if (format >= 0 && format <= 3 && !_nc_globals.slk_format) {
	_nc_globals.slk_format = 1 + format;
	code = _nc_ripoffline(-SLK_LINES(_nc_globals.slk_format), _nc_slk_initialize);
    }
    returnCode(code);
}