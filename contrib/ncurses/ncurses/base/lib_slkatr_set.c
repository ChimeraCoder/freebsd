
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
 *  Author:  Juergen Pfeifer, 1998                                          *
 *     and:  Thomas E. Dickey 2005                                          *
 ****************************************************************************/

/*
 *	lib_slkatr_set.c
 *	Soft key routines.
 *	Set the label's attributes
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkatr_set.c,v 1.10 2005/01/28 21:11:53 tom Exp $")

NCURSES_EXPORT(int)
slk_attr_set(const attr_t attr, short color_pair_number, void *opts)
{
    T((T_CALLED("slk_attr_set(%s,%d)"), _traceattr(attr), color_pair_number));

    if (SP != 0 && SP->_slk != 0 && !opts &&
	color_pair_number >= 0 && color_pair_number < COLOR_PAIRS) {
	TR(TRACE_ATTRS, ("... current %s", _tracech_t(CHREF(SP->_slk->attr))));
	SetAttr(SP->_slk->attr, attr);
	if (color_pair_number > 0) {
	    SetPair(SP->_slk->attr, color_pair_number);
	}
	TR(TRACE_ATTRS, ("new attribute is %s", _tracech_t(CHREF(SP->_slk->attr))));
	returnCode(OK);
    } else
	returnCode(ERR);
}