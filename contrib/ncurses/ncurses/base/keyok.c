
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
 *  Author: Thomas E. Dickey            1997-on                             *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: keyok.c,v 1.7 2006/12/30 16:22:33 tom Exp $")

/*
 * Enable (or disable) ncurses' interpretation of a keycode by adding (or
 * removing) the corresponding 'tries' entry.
 *
 * Do this by storing a second tree of tries, which records the disabled keys. 
 * The simplest way to copy is to make a function that returns the string (with
 * nulls set to 0200), then use that to reinsert the string into the
 * corresponding tree.
 */

NCURSES_EXPORT(int)
keyok(int c, bool flag)
{
    int code = ERR;
    int count = 0;
    char *s;

    T((T_CALLED("keyok(%d,%d)"), c, flag));
    if (c >= 0) {
	unsigned ch = (unsigned) c;
	if (flag) {
	    while ((s = _nc_expand_try(SP->_key_ok, ch, &count, 0)) != 0
		   && _nc_remove_key(&(SP->_key_ok), ch)) {
		code = _nc_add_to_try(&(SP->_keytry), s, ch);
		free(s);
		count = 0;
		if (code != OK)
		    break;
	    }
	} else {
	    while ((s = _nc_expand_try(SP->_keytry, ch, &count, 0)) != 0
		   && _nc_remove_key(&(SP->_keytry), ch)) {
		code = _nc_add_to_try(&(SP->_key_ok), s, ch);
		free(s);
		count = 0;
		if (code != OK)
		    break;
	    }
	}
    }
    returnCode(code);
}