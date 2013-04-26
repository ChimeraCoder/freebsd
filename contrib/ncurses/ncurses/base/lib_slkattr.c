
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
 *	lib_slkattr.c
 *	Soft key routines.
 *      Fetch the labels attributes
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slkattr.c,v 1.6 2005/01/08 21:44:28 tom Exp $")

NCURSES_EXPORT(attr_t)
slk_attr(void)
{
    T((T_CALLED("slk_attr()")));

    if (SP != 0 && SP->_slk != 0) {
	attr_t result = AttrOf(SP->_slk->attr) & ALL_BUT_COLOR;
	int pair = GetPair(SP->_slk->attr);

	result |= COLOR_PAIR(pair);
	returnAttr(result);
    } else
	returnAttr(0);
}