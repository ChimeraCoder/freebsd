
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
 * Authors: Sven Verdoolaege and Thomas Dickey 2001,2002                    *
 ****************************************************************************/

/*
**	lib_box_set.c
**
**	The routine wborder_set().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_box_set.c,v 1.4 2003/12/06 18:02:13 tom Exp $")

NCURSES_EXPORT(int)
wborder_set(WINDOW *win,
	    const ARG_CH_T ls, const ARG_CH_T rs,
	    const ARG_CH_T ts, const ARG_CH_T bs,
	    const ARG_CH_T tl, const ARG_CH_T tr,
	    const ARG_CH_T bl, const ARG_CH_T br)
{
    NCURSES_SIZE_T i;
    NCURSES_SIZE_T endx, endy;
    NCURSES_CH_T wls, wrs, wts, wbs, wtl, wtr, wbl, wbr;

    T((T_CALLED("wborder(%p,%s,%s,%s,%s,%s,%s,%s,%s)"),
       win,
       _tracech_t2(1, ls),
       _tracech_t2(2, rs),
       _tracech_t2(3, ts),
       _tracech_t2(4, bs),
       _tracech_t2(5, tl),
       _tracech_t2(6, tr),
       _tracech_t2(7, bl),
       _tracech_t2(8, br)));

    if (!win)
	returnCode(ERR);

#define RENDER_WITH_DEFAULT(ch,def) w ##ch = _nc_render(win, (ch == 0) ? *(const ARG_CH_T)def : *ch)

    RENDER_WITH_DEFAULT(ls, WACS_VLINE);
    RENDER_WITH_DEFAULT(rs, WACS_VLINE);
    RENDER_WITH_DEFAULT(ts, WACS_HLINE);
    RENDER_WITH_DEFAULT(bs, WACS_HLINE);
    RENDER_WITH_DEFAULT(tl, WACS_ULCORNER);
    RENDER_WITH_DEFAULT(tr, WACS_URCORNER);
    RENDER_WITH_DEFAULT(bl, WACS_LLCORNER);
    RENDER_WITH_DEFAULT(br, WACS_LRCORNER);

    T(("using %s, %s, %s, %s, %s, %s, %s, %s",
       _tracech_t2(1, CHREF(wls)),
       _tracech_t2(2, CHREF(wrs)),
       _tracech_t2(3, CHREF(wts)),
       _tracech_t2(4, CHREF(wbs)),
       _tracech_t2(5, CHREF(wtl)),
       _tracech_t2(6, CHREF(wtr)),
       _tracech_t2(7, CHREF(wbl)),
       _tracech_t2(8, CHREF(wbr))));

    endx = win->_maxx;
    endy = win->_maxy;

    for (i = 0; i <= endx; i++) {
	win->_line[0].text[i] = wts;
	win->_line[endy].text[i] = wbs;
    }
    win->_line[endy].firstchar = win->_line[0].firstchar = 0;
    win->_line[endy].lastchar = win->_line[0].lastchar = endx;

    for (i = 0; i <= endy; i++) {
	win->_line[i].text[0] = wls;
	win->_line[i].text[endx] = wrs;
	win->_line[i].firstchar = 0;
	win->_line[i].lastchar = endx;
    }
    win->_line[0].text[0] = wtl;
    win->_line[0].text[endx] = wtr;
    win->_line[endy].text[0] = wbl;
    win->_line[endy].text[endx] = wbr;

    _nc_synchook(win);
    returnCode(OK);
}