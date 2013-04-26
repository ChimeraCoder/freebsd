
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
 *  Author: Thomas E. Dickey                 1999-on                        *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: keybound.c,v 1.7 2006/06/17 18:19:24 tom Exp $")

/*
 * Returns the count'th string definition which is associated with the
 * given keycode.  The result is malloc'd, must be freed by the caller.
 */
NCURSES_EXPORT(char *)
keybound(int code, int count)
{
    char *result = 0;

    T((T_CALLED("keybound(%d,%d)"), code, count));
    if (SP != 0 && code >= 0) {
	result = _nc_expand_try(SP->_keytry, (unsigned) code, &count, 0);
    }
    returnPtr(result);
}