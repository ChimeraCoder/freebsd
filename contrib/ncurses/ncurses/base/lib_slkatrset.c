
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
 *	lib_slkatrset.c
 *	Soft key routines.
 *      Set the labels attributes
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkatrset.c,v 1.7 2005/01/08 21:46:47 tom Exp $")

NCURSES_EXPORT(int)
slk_attrset(const chtype attr)
{
    T((T_CALLED("slk_attrset(%s)"), _traceattr(attr)));

    if (SP != 0 && SP->_slk != 0) {
	SetAttr(SP->_slk->attr, attr);
	returnCode(OK);
    } else
	returnCode(ERR);
}