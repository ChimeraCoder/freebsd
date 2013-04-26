
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
 *	beep.c
 *
 *	The routine beep().
 *
 */

#include <curses.priv.h>
#include <term.h>		/* beep, flash */

MODULE_ID("$Id: lib_beep.c,v 1.10 2005/04/09 15:20:04 tom Exp $")

/*
 *	beep()
 *
 *	Sound the current terminal's audible bell if it has one.   If not,
 *	flash the screen if possible.
 *
 */

NCURSES_EXPORT(int)
beep(void)
{
    int res = ERR;

    T((T_CALLED("beep()")));

    /* FIXME: should make sure that we are not in altchar mode */
    if (cur_term == 0) {
	res = ERR;
    } else if (bell) {
	TPUTS_TRACE("bell");
	res = putp(bell);
	_nc_flush();
    } else if (flash_screen) {
	TPUTS_TRACE("flash_screen");
	res = putp(flash_screen);
	_nc_flush();
    }

    returnCode(res);
}