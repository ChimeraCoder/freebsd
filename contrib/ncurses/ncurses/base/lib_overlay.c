
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
**	lib_overlay.c
**
**	The routines overlay(), copywin(), and overwrite().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_overlay.c,v 1.27 2008/06/07 23:30:34 tom Exp $")

static int
overlap(const WINDOW *const src, WINDOW *const dst, int const flag)
{
    int rc = ERR;
    int sx1, sy1, sx2, sy2;
    int dx1, dy1, dx2, dy2;
    int sminrow, smincol;
    int dminrow, dmincol;
    int dmaxrow, dmaxcol;

    T((T_CALLED("overlap(%p,%p,%d)"), src, dst, flag));

    if (src != 0 && dst != 0) {
	_nc_lock_global(curses);

	T(("src : begy %ld, begx %ld, maxy %ld, maxx %ld",
	   (long) src->_begy,
	   (long) src->_begx,
	   (long) src->_maxy,
	   (long) src->_maxx));
	T(("dst : begy %ld, begx %ld, maxy %ld, maxx %ld",
	   (long) dst->_begy,
	   (long) dst->_begx,
	   (long) dst->_maxy,
	   (long) dst->_maxx));

	sx1 = src->_begx;
	sy1 = src->_begy;
	sx2 = sx1 + src->_maxx;
	sy2 = sy1 + src->_maxy;

	dx1 = dst->_begx;
	dy1 = dst->_begy;
	dx2 = dx1 + dst->_maxx;
	dy2 = dy1 + dst->_maxy;

	if (dx2 >= sx1 && dx1 <= sx2 && dy2 >= sy1 && dy1 <= sy2) {
	    sminrow = max(sy1, dy1) - sy1;
	    smincol = max(sx1, dx1) - sx1;
	    dminrow = max(sy1, dy1) - dy1;
	    dmincol = max(sx1, dx1) - dx1;
	    dmaxrow = min(sy2, dy2) - dy1;
	    dmaxcol = min(sx2, dx2) - dx1;

	    rc = copywin(src, dst,
			 sminrow, smincol,
			 dminrow, dmincol,
			 dmaxrow, dmaxcol,
			 flag);
	}
	_nc_unlock_global(curses);
    }
    returnCode(rc);
}

/*
**
**	overlay(win1, win2)
**
**
**	overlay() writes the overlapping area of win1 behind win2
**	on win2 non-destructively.
**
**/

NCURSES_EXPORT(int)
overlay(const WINDOW *win1, WINDOW *win2)
{
    T((T_CALLED("overlay(%p,%p)"), win1, win2));
    returnCode(overlap(win1, win2, TRUE));
}

/*
**
**	overwrite(win1, win2)
**
**
**	overwrite() writes the overlapping area of win1 behind win2
**	on win2 destructively.
**
**/

NCURSES_EXPORT(int)
overwrite(const WINDOW *win1, WINDOW *win2)
{
    T((T_CALLED("overwrite(%p,%p)"), win1, win2));
    returnCode(overlap(win1, win2, FALSE));
}

NCURSES_EXPORT(int)
copywin(const WINDOW *src, WINDOW *dst,
	int sminrow, int smincol,
	int dminrow, int dmincol,
	int dmaxrow, int dmaxcol,
	int over)
{
    int rc = ERR;
    int sx, sy, dx, dy;
    bool touched;
    attr_t bk;
    attr_t mask;

    T((T_CALLED("copywin(%p, %p, %d, %d, %d, %d, %d, %d, %d)"),
       src, dst, sminrow, smincol, dminrow, dmincol, dmaxrow, dmaxcol, over));

    if (src && dst) {
	_nc_lock_global(curses);

	bk = AttrOf(dst->_nc_bkgd);
	mask = ~(attr_t) ((bk & A_COLOR) ? A_COLOR : 0);

	/* make sure rectangle exists in source */
	if ((sminrow + dmaxrow - dminrow) <= (src->_maxy + 1) &&
	    (smincol + dmaxcol - dmincol) <= (src->_maxx + 1)) {

	    T(("rectangle exists in source"));

	    /* make sure rectangle fits in destination */
	    if (dmaxrow <= dst->_maxy && dmaxcol <= dst->_maxx) {

		T(("rectangle fits in destination"));

		for (dy = dminrow, sy = sminrow;
		     dy <= dmaxrow;
		     sy++, dy++) {

		    touched = FALSE;
		    for (dx = dmincol, sx = smincol;
			 dx <= dmaxcol;
			 sx++, dx++) {
			if (over) {
			    if ((CharOf(src->_line[sy].text[sx]) != L(' ')) &&
				(!CharEq(dst->_line[dy].text[dx],
					 src->_line[sy].text[sx]))) {
				dst->_line[dy].text[dx] =
				    src->_line[sy].text[sx];
				SetAttr(dst->_line[dy].text[dx],
					((AttrOf(src->_line[sy].text[sx]) &
					  mask) | bk));
				touched = TRUE;
			    }
			} else {
			    if (!CharEq(dst->_line[dy].text[dx],
					src->_line[sy].text[sx])) {
				dst->_line[dy].text[dx] =
				    src->_line[sy].text[sx];
				touched = TRUE;
			    }
			}
		    }
		    if (touched) {
			touchline(dst, dminrow, (dmaxrow - dminrow + 1));
		    }
		}
		T(("finished copywin"));
		rc = OK;
	    }
	}
	_nc_unlock_global(curses);
    }
    returnCode(rc);
}