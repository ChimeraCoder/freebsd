
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
 *	echo.c
 *
 *	Routines:
 *		echo()
 *		noecho()
 *
 */

#include <curses.priv.h>

MODULE_ID("$Id: lib_echo.c,v 1.5 2000/12/10 02:43:27 tom Exp $")

NCURSES_EXPORT(int)
echo(void)
{
    T((T_CALLED("echo()")));
    SP->_echo = TRUE;
    returnCode(OK);
}

NCURSES_EXPORT(int)
noecho(void)
{
    T((T_CALLED("noecho()")));
    SP->_echo = FALSE;
    returnCode(OK);
}