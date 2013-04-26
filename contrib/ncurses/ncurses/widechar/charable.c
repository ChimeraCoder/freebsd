
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
**	Support functions for wide/narrow conversion.
*/

#include <curses.priv.h>

MODULE_ID("$Id: charable.c,v 1.5 2008/07/05 20:51:41 tom Exp $")

NCURSES_EXPORT(bool) _nc_is_charable(wchar_t ch)
{
    bool result;
#if HAVE_WCTOB
    result = (wctob((wint_t) ch) == (int) ch);
#else
    result = (_nc_to_char(ch) >= 0);
#endif
    return result;
}

NCURSES_EXPORT(int) _nc_to_char(wint_t ch)
{
    int result;
#if HAVE_WCTOB
    result = wctob(ch);
#elif HAVE_WCTOMB
    char temp[MB_LEN_MAX];
    result = wctomb(temp, ch);
    if (strlen(temp) == 1)
	result = UChar(temp[0]);
    else
	result = -1;
#endif
    return result;
}

NCURSES_EXPORT(wint_t) _nc_to_widechar(int ch)
{
    wint_t result;
#if HAVE_BTOWC
    result = btowc(ch);
#elif HAVE_MBTOWC
    wchar_t convert;
    char temp[2];
    temp[0] = ch;
    temp[1] = '\0';
    if (mbtowc(&convert, temp, 1) >= 0)
	result = convert;
    else
	result = WEOF;
#endif
    return result;
}