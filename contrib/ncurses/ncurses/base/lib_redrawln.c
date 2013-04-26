
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1997                        *
 ****************************************************************************/

/*
 *	lib_redrawln.c
 *
 *	The routine wredrawln().
 *
 */

#include <curses.priv.h>

MODULE_ID("$Id: lib_redrawln.c,v 1.12 2007/10/13 20:08:56 tom Exp $")

NCURSES_EXPORT(int)
wredrawln(WINDOW *win, int beg, int num)
{
    int i;
    int end;
    size_t len;

    T((T_CALLED("wredrawln(%p,%d,%d)"), win, beg, num));

    if (win == 0)
	returnCode(ERR);

    if (beg < 0)
	beg = 0;

    if (touchline(win, beg, num) == ERR)
	returnCode(ERR);

    if (touchline(curscr, beg + win->_begy, num) == ERR)
	returnCode(ERR);

    end = beg + num;
    if (end > curscr->_maxy + 1)
	end = curscr->_maxy + 1;
    if (end > win->_maxy + 1)
	end = win->_maxy + 1;

    len = (win->_maxx + 1);
    if (len > (size_t) (curscr->_maxx + 1))
	len = (size_t) (curscr->_maxx + 1);
    len *= sizeof(curscr->_line[0].text[0]);

    for (i = beg; i < end; i++) {
	int crow = i + win->_begy;

	memset(curscr->_line[crow].text + win->_begx, 0, len);
	_nc_make_oldhash(crow);
    }

    returnCode(OK);
}