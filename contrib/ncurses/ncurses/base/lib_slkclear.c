
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
 *     and: Juergen Pfeifer                         1996-1999               *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
 *	lib_slkclear.c
 *	Soft key routines.
 *      Remove soft labels from the screen.
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkclear.c,v 1.10 2007/12/29 17:51:47 tom Exp $")

NCURSES_EXPORT(int)
slk_clear(void)
{
    int rc = ERR;

    T((T_CALLED("slk_clear()")));

    if (SP != NULL && SP->_slk != NULL) {
	SP->_slk->hidden = TRUE;
	/* For simulated SLK's it looks much more natural to
	   inherit those attributes from the standard screen */
	SP->_slk->win->_nc_bkgd = stdscr->_nc_bkgd;
	WINDOW_ATTRS(SP->_slk->win) = WINDOW_ATTRS(stdscr);
	if (SP->_slk->win == stdscr) {
	    rc = OK;
	} else {
	    werase(SP->_slk->win);
	    rc = wrefresh(SP->_slk->win);
	}
    }
    returnCode(rc);
}