
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
**	lib_touch.c
**
**	   The routines	untouchwin(),
**			wtouchln(),
**			is_linetouched()
**			is_wintouched().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_touch.c,v 1.9 2000/12/10 02:43:27 tom Exp $")

NCURSES_EXPORT(bool)
is_linetouched(WINDOW *win, int line)
{
    T((T_CALLED("is_linetouched(%p,%d)"), win, line));

    /* XSI doesn't define any error */
    if (!win || (line > win->_maxy) || (line < 0))
	returnCode((bool) ERR);

    returnCode(win->_line[line].firstchar != _NOCHANGE ? TRUE : FALSE);
}

NCURSES_EXPORT(bool)
is_wintouched(WINDOW *win)
{
    int i;

    T((T_CALLED("is_wintouched(%p)"), win));

    if (win)
	for (i = 0; i <= win->_maxy; i++)
	    if (win->_line[i].firstchar != _NOCHANGE)
		returnCode(TRUE);
    returnCode(FALSE);
}

NCURSES_EXPORT(int)
wtouchln(WINDOW *win, int y, int n, int changed)
{
    int i;

    T((T_CALLED("wtouchln(%p,%d,%d,%d)"), win, y, n, changed));

    if (!win || (n < 0) || (y < 0) || (y > win->_maxy))
	returnCode(ERR);

    for (i = y; i < y + n; i++) {
	if (i > win->_maxy)
	    break;
	win->_line[i].firstchar = changed ? 0 : _NOCHANGE;
	win->_line[i].lastchar = changed ? win->_maxx : _NOCHANGE;
    }
    returnCode(OK);
}