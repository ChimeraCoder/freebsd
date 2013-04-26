
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
**	lib_mvwin.c
**
**	The routine mvwin().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_mvwin.c,v 1.14 2006/02/25 22:53:46 tom Exp $")

NCURSES_EXPORT(int)
mvwin(WINDOW *win, int by, int bx)
{
    T((T_CALLED("mvwin(%p,%d,%d)"), win, by, bx));

    if (!win || (win->_flags & _ISPAD))
	returnCode(ERR);

    /*
     * mvwin() should only modify the indices.  See test/demo_menus.c and
     * test/movewindow.c for examples.
     */
#if 0
    /* Copying subwindows is allowed, but it is expensive... */
    if (win->_flags & _SUBWIN) {
	int err = ERR;
	WINDOW *parent = win->_parent;
	if (parent) {		/* Now comes the complicated and costly part, you should really
				 * try to avoid to move subwindows. Because a subwindow shares
				 * the text buffers with its parent, one can't do a simple
				 * memmove of the text buffers. One has to create a copy, then
				 * to relocate the subwindow and then to do a copy.
				 */
	    if ((by - parent->_begy == win->_pary) &&
		(bx - parent->_begx == win->_parx))
		err = OK;	/* we don't actually move */
	    else {
		WINDOW *clone = dupwin(win);
		if (clone) {
		    /* now we have the clone, so relocate win */

		    werase(win);	/* Erase the original place     */
		    /* fill with parents background */
		    wbkgrnd(win, CHREF(parent->_nc_bkgd));
		    wsyncup(win);	/* Tell the parent(s)           */

		    err = mvderwin(win,
				   by - parent->_begy,
				   bx - parent->_begx);
		    if (err != ERR) {
			err = copywin(clone, win,
				      0, 0, 0, 0, win->_maxy, win->_maxx, 0);
			if (ERR != err)
			    wsyncup(win);
		    }
		    if (ERR == delwin(clone))
			err = ERR;
		}
	    }
	}
	returnCode(err);
    }
#endif

    if (by + win->_maxy > screen_lines - 1
	|| bx + win->_maxx > screen_columns - 1
	|| by < 0
	|| bx < 0)
	returnCode(ERR);

    /*
     * Whether or not the window is moved, touch the window's contents so
     * that a following call to 'wrefresh()' will paint the window at the
     * new location.  This ensures that if the caller has refreshed another
     * window at the same location, that this one will be displayed.
     */
    win->_begy = by;
    win->_begx = bx;
    returnCode(touchwin(win));
}