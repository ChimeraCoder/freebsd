
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
 *  Author: Thomas E. Dickey 2002                                           *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: lib_erasewchar.c,v 1.1 2002/05/11 20:38:06 tom Exp $")

/*
 *	erasewchar()
 *
 *	Return erase character as given in cur_term->Ottyb.
 *
 */

NCURSES_EXPORT(int)
erasewchar(wchar_t * wch)
{
    int value;
    int result = ERR;

    T((T_CALLED("erasewchar()")));
    if ((value = erasechar()) != ERR) {
	*wch = value;
	result = OK;
    }
    returnCode(result);
}

/*
 *	killwchar()
 *
 *	Return kill character as given in cur_term->Ottyb.
 *
 */

NCURSES_EXPORT(int)
killwchar(wchar_t * wch)
{
    int value;
    int result = ERR;

    T((T_CALLED("killwchar()")));
    if ((value = killchar()) != ERR) {
	*wch = value;
	result = OK;
    }
    returnCode(result);
}