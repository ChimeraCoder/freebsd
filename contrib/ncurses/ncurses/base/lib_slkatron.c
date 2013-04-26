
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
 *  Author:  Juergen Pfeifer, 1997                                          *
 *     and:  Thomas E. Dickey 2005                                          *
 ****************************************************************************/

/*
 *	lib_slkatron.c
 *	Soft key routines.
 *      Switch on labels attributes
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkatron.c,v 1.8 2005/01/08 23:02:01 tom Exp $")

NCURSES_EXPORT(int)
slk_attron(const chtype attr)
{
    T((T_CALLED("slk_attron(%s)"), _traceattr(attr)));

    if (SP != 0 && SP->_slk != 0) {
	TR(TRACE_ATTRS, ("... current %s", _tracech_t(CHREF(SP->_slk->attr))));
	AddAttr(SP->_slk->attr, attr);
	if ((attr & A_COLOR) != 0) {
	    SetPair(SP->_slk->attr, PAIR_NUMBER(attr));
	}
	TR(TRACE_ATTRS, ("new attribute is %s", _tracech_t(CHREF(SP->_slk->attr))));
	returnCode(OK);
    } else
	returnCode(ERR);
}