
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
**	lib_add_wch.c
**
**	The routine wadd_wch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_add_wch.c,v 1.6 2006/12/02 21:19:17 tom Exp $")

NCURSES_EXPORT(int)
wadd_wch(WINDOW *win, const cchar_t *wch)
{
    PUTC_DATA;
    int n;
    int code = ERR;

    TR(TRACE_VIRTPUT | TRACE_CCALLS, (T_CALLED("wadd_wch(%p, %s)"), win,
				      _tracech_t(wch)));

    if (win != 0) {
	PUTC_INIT;
	for (PUTC_i = 0; PUTC_i < CCHARW_MAX; ++PUTC_i) {
	    attr_t attrs = (wch->attr & A_ATTRIBUTES);

	    if ((PUTC_ch = wch->chars[PUTC_i]) == L'\0')
		break;
	    if ((PUTC_n = wcrtomb(PUTC_buf, PUTC_ch, &PUT_st)) <= 0) {
		code = ERR;
		if (is8bits(PUTC_ch))
		    code = waddch(win, UChar(PUTC_ch) | attrs);
		break;
	    }
	    for (n = 0; n < PUTC_n; n++) {
		if ((code = waddch(win, UChar(PUTC_buf[n]) | attrs)) == ERR) {
		    break;
		}
	    }
	    if (code == ERR)
		break;
	}
    }

    TR(TRACE_VIRTPUT | TRACE_CCALLS, (T_RETURN("%d"), code));
    return (code);
}

NCURSES_EXPORT(int)
wecho_wchar(WINDOW *win, const cchar_t *wch)
{
    PUTC_DATA;
    int n;
    int code = ERR;

    TR(TRACE_VIRTPUT | TRACE_CCALLS, (T_CALLED("wecho_wchar(%p, %s)"), win,
				      _tracech_t(wch)));

    if (win != 0) {
	PUTC_INIT;
	for (PUTC_i = 0; PUTC_i < CCHARW_MAX; ++PUTC_i) {
	    attr_t attrs = (wch->attr & A_ATTRIBUTES);

	    if ((PUTC_ch = wch->chars[PUTC_i]) == L'\0')
		break;
	    if ((PUTC_n = wcrtomb(PUTC_buf, PUTC_ch, &PUT_st)) <= 0) {
		code = ERR;
		if (is8bits(PUTC_ch))
		    code = waddch(win, UChar(PUTC_ch) | attrs);
		break;
	    }
	    for (n = 0; n < PUTC_n; n++) {
		if ((code = waddch(win, UChar(PUTC_buf[n]) | attrs)) == ERR) {
		    break;
		}
	    }
	    if (code == ERR)
		break;
	}
	wrefresh(win);
    }

    TR(TRACE_VIRTPUT | TRACE_CCALLS, (T_RETURN("%d"), code));
    return (code);
}