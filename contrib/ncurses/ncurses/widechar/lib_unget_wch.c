
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

/*
**	lib_unget_wch.c
**
**	The routine unget_wch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_unget_wch.c,v 1.10 2008/06/07 14:50:37 tom Exp $")

/*
 * Wrapper for wcrtomb() which obtains the length needed for the given
 * wide-character 'source'.
 */
NCURSES_EXPORT(size_t)
_nc_wcrtomb(char *target, wchar_t source, mbstate_t * state)
{
    int result;

    if (target == 0) {
	wchar_t temp[2];
	const wchar_t *tempp = temp;
	temp[0] = source;
	temp[1] = 0;
	result = wcsrtombs(NULL, &tempp, 0, state);
    } else {
	result = wcrtomb(target, source, state);
    }
    if (!isEILSEQ(result) && (result == 0))
	result = 1;
    return result;
}

NCURSES_EXPORT(int)
unget_wch(const wchar_t wch)
{
    int result = OK;
    mbstate_t state;
    size_t length;
    int n;

    T((T_CALLED("unget_wch(%#lx)"), (unsigned long) wch));

    init_mb(state);
    length = _nc_wcrtomb(0, wch, &state);

    if (length != (size_t) (-1)
	&& length != 0) {
	char *string;

	if ((string = (char *) malloc(length)) != 0) {
	    init_mb(state);
	    wcrtomb(string, wch, &state);

	    for (n = (int) (length - 1); n >= 0; --n) {
		if (_nc_ungetch(SP, string[n]) != OK) {
		    result = ERR;
		    break;
		}
	    }
	    free(string);
	} else {
	    result = ERR;
	}
    } else {
	result = ERR;
    }

    returnCode(result);
}