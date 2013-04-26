
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
 *     and: Juergen Pfeifer                         1996-on                 *
 *     and: Thomas E. Dickey                                                *
 ****************************************************************************/

/*
 *	lib_slkrefr.c
 *	Write SLK window to the (virtual) screen.
 */
#include <curses.priv.h>
#include <term.h>		/* num_labels, label_*, plab_norm */

MODULE_ID("$Id: lib_slkrefr.c,v 1.17 2008/09/27 14:07:53 juergen Exp $")

/*
 * Paint the info line for the PC style SLK emulation.
 */
static void
slk_paint_info(WINDOW *win)
{
    SCREEN *sp = _nc_screen_of(win);

    if (win && sp && (sp->slk_format == 4)) {
	int i;

	mvwhline(win, 0, 0, 0, getmaxx(win));
	wmove(win, 0, 0);

	for (i = 0; i < sp->_slk->maxlab; i++) {
	    mvwprintw(win, 0, sp->_slk->ent[i].ent_x, "F%d", i + 1);
	}
    }
}

/*
 * Write the soft labels to the soft-key window.
 */
static void
slk_intern_refresh(SLK * slk)
{
    int i;
    int fmt = SP->slk_format;

    for (i = 0; i < slk->labcnt; i++) {
	if (slk->dirty || slk->ent[i].dirty) {
	    if (slk->ent[i].visible) {
		if (num_labels > 0 && SLK_STDFMT(fmt)) {
		    if (i < num_labels) {
			TPUTS_TRACE("plab_norm");
			putp(TPARM_2(plab_norm, i + 1, slk->ent[i].form_text));
		    }
		} else {
		    if (fmt == 4)
			slk_paint_info(slk->win);
		    wmove(slk->win, SLK_LINES(fmt) - 1, slk->ent[i].ent_x);
		    if (SP->_slk) {
			wattrset(slk->win, AttrOf(SP->_slk->attr));
		    }
		    waddstr(slk->win, slk->ent[i].form_text);
		    /* if we simulate SLK's, it's looking much more
		       natural to use the current ATTRIBUTE also
		       for the label window */
		    wattrset(slk->win, WINDOW_ATTRS(stdscr));
		}
	    }
	    slk->ent[i].dirty = FALSE;
	}
    }
    slk->dirty = FALSE;

    if (num_labels > 0) {
	if (slk->hidden) {
	    TPUTS_TRACE("label_off");
	    putp(label_off);
	} else {
	    TPUTS_TRACE("label_on");
	    putp(label_on);
	}
    }
}

/*
 * Refresh the soft labels.
 */
NCURSES_EXPORT(int)
slk_noutrefresh(void)
{
    T((T_CALLED("slk_noutrefresh()")));

    if (SP == NULL || SP->_slk == NULL)
	returnCode(ERR);
    if (SP->_slk->hidden)
	returnCode(OK);
    slk_intern_refresh(SP->_slk);

    returnCode(wnoutrefresh(SP->_slk->win));
}

/*
 * Refresh the soft labels.
 */
NCURSES_EXPORT(int)
slk_refresh(void)
{
    T((T_CALLED("slk_refresh()")));

    if (SP == NULL || SP->_slk == NULL)
	returnCode(ERR);
    if (SP->_slk->hidden)
	returnCode(OK);
    slk_intern_refresh(SP->_slk);

    returnCode(wrefresh(SP->_slk->win));
}