
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

MODULE_ID("$Id: lib_pecho_wchar.c,v 1.1 2004/01/03 21:42:01 tom Exp $")

NCURSES_EXPORT(int)
pecho_wchar(WINDOW *pad, const cchar_t * wch)
{
    T((T_CALLED("pecho_wchar(%p, %s)"), pad, _tracech_t(wch)));

    if (pad == 0)
	returnCode(ERR);

    if (!(pad->_flags & _ISPAD))
	returnCode(wecho_wchar(pad, wch));

    wadd_wch(pad, wch);
    prefresh(pad, pad->_pad._pad_y,
	     pad->_pad._pad_x,
	     pad->_pad._pad_top,
	     pad->_pad._pad_left,
	     pad->_pad._pad_bottom,
	     pad->_pad._pad_right);

    returnCode(OK);
}