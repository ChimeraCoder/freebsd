
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1999                        *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: version.c,v 1.6 2005/01/02 01:23:54 tom Exp $")

NCURSES_EXPORT(const char *)
curses_version(void)
{
    T((T_CALLED("curses_version()")));
    returnCPtr("ncurses " NCURSES_VERSION_STRING);
}