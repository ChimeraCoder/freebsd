
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
 ****************************************************************************/

/*
**	lib_getstr.c
**
**	The routine wgetstr().
**
*/

#include <curses.priv.h>
#include <term.h>

MODULE_ID("$Id: lib_getstr.c,v 1.27 2008/08/16 19:20:04 tom Exp $")

/*
 * This wipes out the last character, no matter whether it was a tab, control
 * or other character, and handles reverse wraparound.
 */
static char *
WipeOut(WINDOW *win, int y, int x, char *first, char *last, bool echoed)
{
    if (last > first) {
	*--last = '\0';
	if (echoed) {
	    int y1 = win->_cury;
	    int x1 = win->_curx;

	    wmove(win, y, x);
	    waddstr(win, first);
	    getyx(win, y, x);
	    while (win->_cury < y1
		   || (win->_cury == y1 && win->_curx < x1))
		waddch(win, (chtype) ' ');

	    wmove(win, y, x);
	}
    }
    return last;
}

NCURSES_EXPORT(int)
wgetnstr_events(WINDOW *win,
		char *str,
		int maxlen,
		EVENTLIST_1st(_nc_eventlist * evl))
{
    SCREEN *sp = _nc_screen_of(win);
    TTY buf;
    bool oldnl, oldecho, oldraw, oldcbreak;
    char erasec;
    char killc;
    char *oldstr;
    int ch;
    int y, x;

    T((T_CALLED("wgetnstr(%p,%p, %d)"), win, str, maxlen));

    if (!win)
	returnCode(ERR);

    _nc_get_tty_mode(&buf);

    oldnl = sp->_nl;
    oldecho = sp->_echo;
    oldraw = sp->_raw;
    oldcbreak = sp->_cbreak;
    nl();
    noecho();
    noraw();
    cbreak();

    erasec = erasechar();
    killc = killchar();

    oldstr = str;
    getyx(win, y, x);

    if (is_wintouched(win) || (win->_flags & _HASMOVED))
	wrefresh(win);

    while ((ch = wgetch_events(win, evl)) != ERR) {
	/*
	 * Some terminals (the Wyse-50 is the most common) generate
	 * a \n from the down-arrow key.  With this logic, it's the
	 * user's choice whether to set kcud=\n for wgetch();
	 * terminating *getstr() with \n should work either way.
	 */
	if (ch == '\n'
	    || ch == '\r'
	    || ch == KEY_DOWN
	    || ch == KEY_ENTER) {
	    if (oldecho == TRUE
		&& win->_cury == win->_maxy
		&& win->_scroll)
		wechochar(win, (chtype) '\n');
	    break;
	}
#ifdef KEY_EVENT
	if (ch == KEY_EVENT)
	    break;
#endif
#ifdef KEY_RESIZE
	if (ch == KEY_RESIZE)
	    break;
#endif
	if (ch == erasec || ch == KEY_LEFT || ch == KEY_BACKSPACE) {
	    if (str > oldstr) {
		str = WipeOut(win, y, x, oldstr, str, oldecho);
	    }
	} else if (ch == killc) {
	    while (str > oldstr) {
		str = WipeOut(win, y, x, oldstr, str, oldecho);
	    }
	} else if (ch >= KEY_MIN
		   || (maxlen >= 0 && str - oldstr >= maxlen)) {
	    beep();
	} else {
	    *str++ = (char) ch;
	    if (oldecho == TRUE) {
		int oldy = win->_cury;
		if (waddch(win, (chtype) ch) == ERR) {
		    /*
		     * We can't really use the lower-right
		     * corner for input, since it'll mess
		     * up bookkeeping for erases.
		     */
		    win->_flags &= ~_WRAPPED;
		    waddch(win, (chtype) ' ');
		    str = WipeOut(win, y, x, oldstr, str, oldecho);
		    continue;
		} else if (win->_flags & _WRAPPED) {
		    /*
		     * If the last waddch forced a wrap &
		     * scroll, adjust our reference point
		     * for erasures.
		     */
		    if (win->_scroll
			&& oldy == win->_maxy
			&& win->_cury == win->_maxy) {
			if (--y <= 0) {
			    y = 0;
			}
		    }
		    win->_flags &= ~_WRAPPED;
		}
		wrefresh(win);
	    }
	}
    }

    win->_curx = 0;
    win->_flags &= ~_WRAPPED;
    if (win->_cury < win->_maxy)
	win->_cury++;
    wrefresh(win);

    /* Restore with a single I/O call, to fix minor asymmetry between
     * raw/noraw, etc.
     */
    sp->_nl = oldnl;
    sp->_echo = oldecho;
    sp->_raw = oldraw;
    sp->_cbreak = oldcbreak;

    _nc_set_tty_mode(&buf);

    *str = '\0';
    if (ch == ERR)
	returnCode(ch);

    T(("wgetnstr returns %s", _nc_visbuf(oldstr)));

#ifdef KEY_EVENT
    if (ch == KEY_EVENT)
	returnCode(ch);
#endif
#ifdef KEY_RESIZE
    if (ch == KEY_RESIZE)
	returnCode(ch);
#endif

    returnCode(OK);
}

#ifdef NCURSES_WGETCH_EVENTS
NCURSES_EXPORT(int)
wgetnstr(WINDOW *win, char *str, int maxlen)
{
    returnCode(wgetnstr_events(win,
			       str,
			       maxlen,
			       EVENTLIST_1st((_nc_eventlist *) 0)));
}
#endif