
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
 *  Author: Thomas E. Dickey 1996-2003                                      *
 *     and: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/*
**	lib_scroll.c
**
**	The routine wscrl(win, n).
**  positive n scroll the window up (ie. move lines down)
**  negative n scroll the window down (ie. move lines up)
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_scroll.c,v 1.26 2006/10/14 20:46:08 tom Exp $")

NCURSES_EXPORT(void)
_nc_scroll_window(WINDOW *win,
		  int const n,
		  NCURSES_SIZE_T const top,
		  NCURSES_SIZE_T const bottom,
		  NCURSES_CH_T blank)
{
    int limit;
    int line;
    int j;
    size_t to_copy = (size_t) (sizeof(NCURSES_CH_T) * (win->_maxx + 1));

    TR(TRACE_MOVE, ("_nc_scroll_window(%p, %d, %ld, %ld)",
		    win, n, (long) top, (long) bottom));

    if (top < 0
	|| bottom < top
	|| bottom > win->_maxy) {
	TR(TRACE_MOVE, ("nothing to scroll"));
	return;
    }

    /*
     * This used to do a line-text pointer-shuffle instead of text copies.
     * That (a) doesn't work when the window is derived and doesn't have
     * its own storage, (b) doesn't save you a lot on modern machines
     * anyway.  Your typical memcpy implementations are coded in
     * assembler using a tight BLT loop; for the size of copies we're
     * talking here, the total execution time is dominated by the one-time
     * setup cost.  So there is no point in trying to be excessively
     * clever -- esr.
     */

    /* shift n lines downwards */
    if (n < 0) {
	limit = top - n;
	for (line = bottom; line >= limit && line >= 0; line--) {
	    TR(TRACE_MOVE, ("...copying %d to %d", line + n, line));
	    memcpy(win->_line[line].text,
		   win->_line[line + n].text,
		   to_copy);
	    if_USE_SCROLL_HINTS(win->_line[line].oldindex =
				win->_line[line + n].oldindex);
	}
	for (line = top; line < limit && line <= win->_maxy; line++) {
	    TR(TRACE_MOVE, ("...filling %d", line));
	    for (j = 0; j <= win->_maxx; j++)
		win->_line[line].text[j] = blank;
	    if_USE_SCROLL_HINTS(win->_line[line].oldindex = _NEWINDEX);
	}
    }

    /* shift n lines upwards */
    if (n > 0) {
	limit = bottom - n;
	for (line = top; line <= limit && line <= win->_maxy; line++) {
	    memcpy(win->_line[line].text,
		   win->_line[line + n].text,
		   to_copy);
	    if_USE_SCROLL_HINTS(win->_line[line].oldindex =
				win->_line[line + n].oldindex);
	}
	for (line = bottom; line > limit && line >= 0; line--) {
	    for (j = 0; j <= win->_maxx; j++)
		win->_line[line].text[j] = blank;
	    if_USE_SCROLL_HINTS(win->_line[line].oldindex = _NEWINDEX);
	}
    }
    touchline(win, top, bottom - top + 1);

    if_WIDEC({
	if (WINDOW_EXT(win, addch_used) != 0) {
	    int next = WINDOW_EXT(win, addch_y) + n;
	    if (next < 0 || next > win->_maxy) {
		TR(TRACE_VIRTPUT,
		   ("Alert discarded multibyte on scroll"));
		WINDOW_EXT(win, addch_y) = 0;
	    } else {
		TR(TRACE_VIRTPUT, ("scrolled working position to %d,%d",
				   WINDOW_EXT(win, addch_y),
				   WINDOW_EXT(win, addch_x)));
		WINDOW_EXT(win, addch_y) = next;
	    }
	}
    })
}

NCURSES_EXPORT(int)
wscrl(WINDOW *win, int n)
{
    T((T_CALLED("wscrl(%p,%d)"), win, n));

    if (!win || !win->_scroll) {
	TR(TRACE_MOVE, ("...scrollok is false"));
	returnCode(ERR);
    }

    if (n != 0) {
	_nc_scroll_window(win, n, win->_regtop, win->_regbottom, win->_nc_bkgd);
	_nc_synchook(win);
    }
    returnCode(OK);
}