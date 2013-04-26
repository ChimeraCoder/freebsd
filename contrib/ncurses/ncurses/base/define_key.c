
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
 *  Author: Thomas E. Dickey                    1997-on                     *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: define_key.c,v 1.13 2006/12/30 23:23:31 tom Exp $")

NCURSES_EXPORT(int)
define_key(const char *str, int keycode)
{
    int code = ERR;

    T((T_CALLED("define_key(%s,%d)"), _nc_visbuf(str), keycode));
    if (SP == 0) {
	code = ERR;
    } else if (keycode > 0) {
	unsigned ukey = (unsigned) keycode;

	if (str != 0) {
	    define_key(str, 0);
	} else if (has_key(keycode)) {
	    while (_nc_remove_key(&(SP->_keytry), ukey))
		code = OK;
	}
	if (str != 0) {
	    if (key_defined(str) == 0) {
		if (_nc_add_to_try(&(SP->_keytry), str, ukey) == OK) {
		    code = OK;
		} else {
		    code = ERR;
		}
	    } else {
		code = ERR;
	    }
	}
    } else {
	while (_nc_remove_string(&(SP->_keytry), str))
	    code = OK;
    }
    returnCode(code);
}