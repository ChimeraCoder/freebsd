
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
 *     and: Juergen Pfeifer                         1997                    *
 *     and: Sven Verdoolaege                        2000                    *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: lib_bkgd.c,v 1.36 2008/03/23 00:09:14 tom Exp $")

/*
 * Set the window's background information.
 */
#if USE_WIDEC_SUPPORT
NCURSES_EXPORT(void)
#else
static NCURSES_INLINE void
#endif
wbkgrndset(WINDOW *win, const ARG_CH_T ch)
{
    T((T_CALLED("wbkgdset(%p,%s)"), win, _tracech_t(ch)));

    if (win) {
	attr_t off = AttrOf(win->_nc_bkgd);
	attr_t on = AttrOf(CHDEREF(ch));

	toggle_attr_off(WINDOW_ATTRS(win), off);
	toggle_attr_on(WINDOW_ATTRS(win), on);

#if NCURSES_EXT_COLORS
	{
	    int pair;

	    if ((pair = GetPair(win->_nc_bkgd)) != 0)
		SET_WINDOW_PAIR(win, 0);
	    if ((pair = GetPair(CHDEREF(ch))) != 0)
		SET_WINDOW_PAIR(win, pair);
	}
#endif

	if (CharOf(CHDEREF(ch)) == L('\0')) {
	    SetChar(win->_nc_bkgd, BLANK_TEXT, AttrOf(CHDEREF(ch)));
	    if_EXT_COLORS(SetPair(win->_nc_bkgd, GetPair(CHDEREF(ch))));
	} else {
	    win->_nc_bkgd = CHDEREF(ch);
	}
#if USE_WIDEC_SUPPORT
	/*
	 * If we're compiled for wide-character support, _bkgrnd is the
	 * preferred location for the background information since it stores
	 * more than _bkgd.  Update _bkgd each time we modify _bkgrnd, so the
	 * macro getbkgd() will work.
	 */
	{
	    cchar_t wch;
	    int tmp;

	    wgetbkgrnd(win, &wch);
	    tmp = _nc_to_char((wint_t) CharOf(wch));

	    win->_bkgd = (((tmp == EOF) ? ' ' : (chtype) tmp)
			  | (AttrOf(wch) & ALL_BUT_COLOR)
			  | COLOR_PAIR(GET_WINDOW_PAIR(win)));
	}
#endif
    }
    returnVoid;
}

NCURSES_EXPORT(void)
wbkgdset(WINDOW *win, chtype ch)
{
    NCURSES_CH_T wch;
    SetChar2(wch, ch);
    wbkgrndset(win, CHREF(wch));
}

/*
 * Set the window's background information and apply it to each cell.
 */
#if USE_WIDEC_SUPPORT
NCURSES_EXPORT(int)
#else
static NCURSES_INLINE int
#undef wbkgrnd
#endif
wbkgrnd(WINDOW *win, const ARG_CH_T ch)
{
    int code = ERR;
    int x, y;
    NCURSES_CH_T new_bkgd = CHDEREF(ch);

    T((T_CALLED("wbkgd(%p,%s)"), win, _tracech_t(ch)));

    if (win) {
	NCURSES_CH_T old_bkgrnd;
	wgetbkgrnd(win, &old_bkgrnd);

	wbkgrndset(win, CHREF(new_bkgd));
	wattrset(win, AttrOf(win->_nc_bkgd));

	for (y = 0; y <= win->_maxy; y++) {
	    for (x = 0; x <= win->_maxx; x++) {
		if (CharEq(win->_line[y].text[x], old_bkgrnd)) {
		    win->_line[y].text[x] = win->_nc_bkgd;
		} else {
		    NCURSES_CH_T wch = win->_line[y].text[x];
		    RemAttr(wch, (~(A_ALTCHARSET | A_CHARTEXT)));
		    win->_line[y].text[x] = _nc_render(win, wch);
		}
	    }
	}
	touchwin(win);
	_nc_synchook(win);
	code = OK;
    }
    returnCode(code);
}

NCURSES_EXPORT(int)
wbkgd(WINDOW *win, chtype ch)
{
    NCURSES_CH_T wch;
    SetChar2(wch, ch);
    return wbkgrnd(win, CHREF(wch));
}