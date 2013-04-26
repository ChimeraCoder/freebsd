
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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

#include <curses.priv.h>
#include <term.h>

MODULE_ID("$Id: lib_dft_fgbg.c,v 1.18 2005/11/26 20:03:38 tom Exp $")

/*
 * Modify the behavior of color-pair 0 so that the library doesn't assume that
 * it is white on black.  This is an extension to XSI curses.
 */
NCURSES_EXPORT(int)
use_default_colors(void)
{
    T((T_CALLED("use_default_colors()")));
    returnCode(assume_default_colors(-1, -1));
}

/*
 * Modify the behavior of color-pair 0 so that the library assumes that it
 * is something specific, possibly not white on black.
 */
NCURSES_EXPORT(int)
assume_default_colors(int fg, int bg)
{
    T((T_CALLED("assume_default_colors(%d,%d)"), fg, bg));

    if (!orig_pair && !orig_colors)
	returnCode(ERR);

    if (initialize_pair)	/* don't know how to handle this */
	returnCode(ERR);

    SP->_default_color = isDefaultColor(fg) || isDefaultColor(bg);
    SP->_has_sgr_39_49 = (tigetflag("AX") == TRUE);
    SP->_default_fg = isDefaultColor(fg) ? COLOR_DEFAULT : (fg & C_MASK);
    SP->_default_bg = isDefaultColor(bg) ? COLOR_DEFAULT : (bg & C_MASK);
    if (SP->_color_pairs != 0) {
	bool save = SP->_default_color;
	SP->_default_color = TRUE;
	init_pair(0, (short) fg, (short) bg);
	SP->_default_color = save;
    }
    returnCode(OK);
}