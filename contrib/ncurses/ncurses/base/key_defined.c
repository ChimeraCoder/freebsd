
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
 *  Author: Thomas E. Dickey, 2003                                          *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: key_defined.c,v 1.6 2006/12/30 23:22:55 tom Exp $")

static int
find_definition(TRIES * tree, const char *str)
{
    TRIES *ptr;
    int result = OK;

    if (str != 0 && *str != '\0') {
	for (ptr = tree; ptr != 0; ptr = ptr->sibling) {
	    if (UChar(*str) == UChar(ptr->ch)) {
		if (str[1] == '\0' && ptr->child != 0) {
		    result = ERR;
		} else if ((result = find_definition(ptr->child, str + 1))
			   == OK) {
		    result = ptr->value;
		} else if (str[1] == '\0') {
		    result = ERR;
		}
	    }
	    if (result != OK)
		break;
	}
    }
    return (result);
}

/*
 * Returns the keycode associated with the given string.  If none is found,
 * return OK.  If the string is only a prefix to other strings, return ERR.
 * Otherwise, return the keycode's value (neither OK/ERR).
 */
NCURSES_EXPORT(int)
key_defined(const char *str)
{
    int code = ERR;

    T((T_CALLED("key_defined(%s)"), _nc_visbuf(str)));
    if (SP != 0 && str != 0) {
	code = find_definition(SP->_keytry, str);
    }

    returnCode(code);
}