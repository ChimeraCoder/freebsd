
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
 *	lib_slklab.c
 *	Soft key routines.
 *      Fetch the label text.
 */
#include <curses.priv.h>

MODULE_ID("$Id: lib_slklab.c,v 1.7 2003/03/29 22:53:48 tom Exp $")

NCURSES_EXPORT(char *)
slk_label(int n)
{
    T((T_CALLED("slk_label(%d)"), n));

    if (SP == NULL || SP->_slk == NULL || n < 1 || n > SP->_slk->labcnt)
	returnPtr(0);
    returnPtr(SP->_slk->ent[n - 1].ent_text);
}