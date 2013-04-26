
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
 *     and: Sven Verdoolaege                        2001                    *
 ****************************************************************************/

/*
**	lib_box.c
**
**	The routine wborder().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_box.c,v 1.22 2005/11/26 15:39:42 tom Exp $")

#if USE_WIDEC_SUPPORT
static NCURSES_INLINE chtype
_my_render(WINDOW *win, chtype ch)
{
    NCURSES_CH_T wch;
    SetChar2(wch, ch);
    wch = _nc_render(win, wch);
    return CharOf(wch) | AttrOf(wch);
}
#define RENDER_WITH_DEFAULT(ch,def) w ## ch = _my_render(win, (ch == 0) ? def : ch)
#else
#define RENDER_WITH_DEFAULT(ch,def) w ## ch = _nc_render(win, (ch == 0) ? def : ch)
#endif

NCURSES_EXPORT(int)
wborder(WINDOW *win,
	chtype ls, chtype rs,
	chtype ts, chtype bs,
	chtype tl, chtype tr,
	chtype bl, chtype br)
{
    NCURSES_SIZE_T i;
    NCURSES_SIZE_T endx, endy;
    chtype wls, wrs, wts, wbs, wtl, wtr, wbl, wbr;

    T((T_CALLED("wborder(%p,%s,%s,%s,%s,%s,%s,%s,%s)"),
       win,
       _tracechtype2(1, ls),
       _tracechtype2(2, rs),
       _tracechtype2(3, ts),
       _tracechtype2(4, bs),
       _tracechtype2(5, tl),
       _tracechtype2(6, tr),
       _tracechtype2(7, bl),
       _tracechtype2(8, br)));

    if (!win)
	returnCode(ERR);

    RENDER_WITH_DEFAULT(ls, ACS_VLINE);
    RENDER_WITH_DEFAULT(rs, ACS_VLINE);
    RENDER_WITH_DEFAULT(ts, ACS_HLINE);
    RENDER_WITH_DEFAULT(bs, ACS_HLINE);
    RENDER_WITH_DEFAULT(tl, ACS_ULCORNER);
    RENDER_WITH_DEFAULT(tr, ACS_URCORNER);
    RENDER_WITH_DEFAULT(bl, ACS_LLCORNER);
    RENDER_WITH_DEFAULT(br, ACS_LRCORNER);

    T(("using %s, %s, %s, %s, %s, %s, %s, %s",
       _tracechtype2(1, wls),
       _tracechtype2(2, wrs),
       _tracechtype2(3, wts),
       _tracechtype2(4, wbs),
       _tracechtype2(5, wtl),
       _tracechtype2(6, wtr),
       _tracechtype2(7, wbl),
       _tracechtype2(8, wbr)));

    endx = win->_maxx;
    endy = win->_maxy;

    for (i = 0; i <= endx; i++) {
	SetChar2(win->_line[0].text[i], wts);
	SetChar2(win->_line[endy].text[i], wbs);
    }
    win->_line[endy].firstchar = win->_line[0].firstchar = 0;
    win->_line[endy].lastchar = win->_line[0].lastchar = endx;

    for (i = 0; i <= endy; i++) {
	SetChar2(win->_line[i].text[0], wls);
	SetChar2(win->_line[i].text[endx], wrs);
	win->_line[i].firstchar = 0;
	win->_line[i].lastchar = endx;
    }
    SetChar2(win->_line[0].text[0], wtl);
    SetChar2(win->_line[0].text[endx], wtr);
    SetChar2(win->_line[endy].text[0], wbl);
    SetChar2(win->_line[endy].text[endx], wbr);

    _nc_synchook(win);
    returnCode(OK);
}