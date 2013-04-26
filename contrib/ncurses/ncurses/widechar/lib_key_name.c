
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

/*
**	lib_key_name.c
**
**	The routine key_name().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_key_name.c,v 1.3 2008/10/11 20:15:14 tom Exp $")

NCURSES_EXPORT(NCURSES_CONST char *)
key_name(wchar_t c)
{
    cchar_t my_cchar;
    wchar_t *my_wchars;
    size_t len;

    /* FIXME: move to _nc_globals */
    static char result[MB_LEN_MAX + 1];

    memset(&my_cchar, 0, sizeof(my_cchar));
    my_cchar.chars[0] = c;
    my_cchar.chars[1] = L'\0';

    my_wchars = wunctrl(&my_cchar);
    len = wcstombs(result, my_wchars, sizeof(result) - 1);
    if (isEILSEQ(len) || (len == 0)) {
	return 0;
    }

    result[len] = '\0';
    return result;
}