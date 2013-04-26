
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
**	lib_wunctrl.c
**
**	The routine wunctrl().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_wunctrl.c,v 1.12 2007/06/12 20:22:32 tom Exp $")

NCURSES_EXPORT(wchar_t *)
wunctrl(cchar_t *wc)
{
    static wchar_t str[CCHARW_MAX + 1], *sp;

    if (Charable(*wc)) {
	const char *p = unctrl((unsigned) _nc_to_char((wint_t) CharOf(*wc)));

	for (sp = str; *p; ++p) {
	    *sp++ = _nc_to_widechar(*p);
	}
	*sp = 0;
	return str;
    } else
	return wc->chars;
}