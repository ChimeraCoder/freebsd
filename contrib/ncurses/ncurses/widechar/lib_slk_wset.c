
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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

/*
 *	lib_slk_wset.c
 *      Set soft label text.
 */
#include <curses.priv.h>

#if HAVE_WCTYPE_H
#include <wctype.h>
#endif

MODULE_ID("$Id: lib_slk_wset.c,v 1.11 2005/01/16 01:03:53 tom Exp $")

NCURSES_EXPORT(int)
slk_wset(int i, const wchar_t *astr, int format)
{
    int result = ERR;
    size_t arglen;
    const wchar_t *str;
    char *mystr;
    mbstate_t state;

    T((T_CALLED("slk_wset(%d, %s, %d)"), i, _nc_viswbuf(astr), format));

    init_mb(state);
    str = astr;
    if ((arglen = wcsrtombs(NULL, &str, 0, &state)) != (size_t) -1) {
	if ((mystr = (char *) _nc_doalloc(0, arglen + 1)) != 0) {
	    str = astr;
	    if (wcsrtombs(mystr, &str, arglen, &state) != (size_t) -1) {
		/* glibc documentation claims that the terminating L'\0'
		 * is written, but it is not...
		 */
		mystr[arglen] = 0;
		result = slk_set(i, mystr, format);
	    }
	    free(mystr);
	}
    }
    returnCode(result);
}