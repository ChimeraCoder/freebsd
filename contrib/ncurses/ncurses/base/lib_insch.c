
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
 *     and: Sven Verdoolaege                                                *
 *     and: Thomas E. Dickey                                                *
 ****************************************************************************/

/*
**	lib_insch.c
**
**	The routine winsch().
**
*/

#include <curses.priv.h>
#include <ctype.h>

MODULE_ID("$Id: lib_insch.c,v 1.25 2008/02/03 00:14:37 tom Exp $")

/*
 * Insert the given character, updating the current location to simplify
 * inserting a string.
 */
NCURSES_EXPORT(int)
_nc_insert_ch(WINDOW *win, chtype ch)
{
    int code = OK;
    NCURSES_CH_T wch;
    int count;
    NCURSES_CONST char *s;

    switch (ch) {
    case '\t':
	for (count = (TABSIZE - (win->_curx % TABSIZE)); count > 0; count--) {
	    if ((code = _nc_insert_ch(win, ' ')) != OK)
		break;
	}
	break;
    case '\n':
    case '\r':
    case '\b':
	SetChar2(wch, ch);
	_nc_waddch_nosync(win, wch);
	break;
    default:
	if (
#if USE_WIDEC_SUPPORT
	       WINDOW_EXT(win, addch_used) == 0 &&
#endif
	       is8bits(ChCharOf(ch)) &&
	       isprint(ChCharOf(ch))) {
	    if (win->_curx <= win->_maxx) {
		struct ldat *line = &(win->_line[win->_cury]);
		NCURSES_CH_T *end = &(line->text[win->_curx]);
		NCURSES_CH_T *temp1 = &(line->text[win->_maxx]);
		NCURSES_CH_T *temp2 = temp1 - 1;

		SetChar2(wch, ch);

		CHANGED_TO_EOL(line, win->_curx, win->_maxx);
		while (temp1 > end)
		    *temp1-- = *temp2--;

		*temp1 = _nc_render(win, wch);
		win->_curx++;
	    }
	} else if (is8bits(ChCharOf(ch)) && iscntrl(ChCharOf(ch))) {
	    s = unctrl(ChCharOf(ch));
	    while (*s != '\0') {
		code = _nc_insert_ch(win, ChAttrOf(ch) | UChar(*s));
		if (code != OK)
		    break;
		++s;
	    }
	}
#if USE_WIDEC_SUPPORT
	else {
	    /*
	     * Handle multibyte characters here
	     */
	    SetChar2(wch, ch);
	    wch = _nc_render(win, wch);
	    count = _nc_build_wch(win, &wch);
	    if (count > 0) {
		code = wins_wch(win, &wch);
	    } else if (count == -1) {
		/* handle EILSEQ */
		if (is8bits(ch)) {
		    s = unctrl(ChCharOf(ch));
		    while (*s != '\0') {
			code = _nc_insert_ch(win, ChAttrOf(ch) | UChar(*s));
			if (code != OK)
			    break;
			++s;
		    }
		} else {
		    code = ERR;
		}
	    }
	}
#endif
	break;
    }
    return code;
}

NCURSES_EXPORT(int)
winsch(WINDOW *win, chtype c)
{
    NCURSES_SIZE_T oy;
    NCURSES_SIZE_T ox;
    int code = ERR;

    T((T_CALLED("winsch(%p, %s)"), win, _tracechtype(c)));

    if (win != 0) {
	oy = win->_cury;
	ox = win->_curx;

	code = _nc_insert_ch(win, c);

	win->_curx = ox;
	win->_cury = oy;
	_nc_synchook(win);
    }
    returnCode(code);
}