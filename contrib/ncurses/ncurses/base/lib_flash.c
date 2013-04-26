
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
 *	flash.c
 *
 *	The routine flash().
 *
 */

#include <curses.priv.h>
#include <term.h>		/* beep, flash */

MODULE_ID("$Id: lib_flash.c,v 1.6 2000/12/10 02:43:27 tom Exp $")

/*
 *	flash()
 *
 *	Flash the current terminal's screen if possible.   If not,
 *	sound the audible bell if one exists.
 *
 */

NCURSES_EXPORT(int)
flash(void)
{
    int res = ERR;

    T((T_CALLED("flash()")));

    /* FIXME: should make sure that we are not in altchar mode */
    if (flash_screen) {
	TPUTS_TRACE("flash_screen");
	res = putp(flash_screen);
	_nc_flush();
    } else if (bell) {
	TPUTS_TRACE("bell");
	res = putp(bell);
	_nc_flush();
    }

    returnCode(res);
}