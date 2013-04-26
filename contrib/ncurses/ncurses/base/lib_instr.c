
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
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
**	lib_instr.c
**
**	The routine winnstr().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_instr.c,v 1.16 2007/07/21 20:18:10 tom Exp $")

NCURSES_EXPORT(int)
winnstr(WINDOW *win, char *str, int n)
{
    int i = 0, row, col;

    T((T_CALLED("winnstr(%p,%p,%d)"), win, str, n));

    if (!str)
	returnCode(0);

    if (win) {
	getyx(win, row, col);

	if (n < 0)
	    n = win->_maxx - win->_curx + 1;

	for (; i < n;) {
#if USE_WIDEC_SUPPORT
	    cchar_t *cell = &(win->_line[row].text[col]);
	    wchar_t *wch;
	    attr_t attrs;
	    short pair;
	    int n2;
	    bool done = FALSE;
	    mbstate_t state;
	    size_t i3, n3;
	    char *tmp;

	    if (!isWidecExt(*cell)) {
		n2 = getcchar(cell, 0, 0, 0, 0);
		if (n2 > 0
		    && (wch = typeCalloc(wchar_t, (unsigned) n2 + 1)) != 0) {
		    if (getcchar(cell, wch, &attrs, &pair, 0) == OK) {

			init_mb(state);
			n3 = wcstombs(0, wch, 0);
			if (isEILSEQ(n3) || (n3 == 0)) {
			    ;
			} else if ((int) (n3 + i) > n) {
			    done = TRUE;
			} else if ((tmp = typeCalloc(char, n3 + 10)) == 0) {
			    done = TRUE;
			} else {
			    init_mb(state);
			    wcstombs(tmp, wch, n3);
			    for (i3 = 0; i3 < n3; ++i3)
				str[i++] = tmp[i3];
			    free(tmp);
			}
		    }
		    free(wch);
		    if (done)
			break;
		}
	    }
#else
	    str[i++] = (char) CharOf(win->_line[row].text[col]);
#endif
	    if (++col > win->_maxx) {
		break;
	    }
	}
    }
    str[i] = '\0';		/* SVr4 does not seem to count the null */
    T(("winnstr returns %s", _nc_visbuf(str)));
    returnCode(i);
}