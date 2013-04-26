
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
 *  Author: Thomas E. Dickey                    1999-on                     *
 ****************************************************************************/

/*
 * free_ttype.c -- allocation functions for TERMTYPE
 *
 *	_nc_free_termtype()
 *	use_extended_names()
 *
 */

#include <curses.priv.h>

#include <tic.h>
#include <term_entry.h>

MODULE_ID("$Id: free_ttype.c,v 1.13 2006/06/25 10:46:02 tom Exp $")

NCURSES_EXPORT(void)
_nc_free_termtype(TERMTYPE *ptr)
{
    T(("_nc_free_termtype(%s)", ptr->term_names));

    FreeIfNeeded(ptr->str_table);
    FreeIfNeeded(ptr->Booleans);
    FreeIfNeeded(ptr->Numbers);
    FreeIfNeeded(ptr->Strings);
#if NCURSES_XNAMES
    FreeIfNeeded(ptr->ext_str_table);
    FreeIfNeeded(ptr->ext_Names);
#endif
    memset(ptr, 0, sizeof(TERMTYPE));
    _nc_free_entry(_nc_head, ptr);
}

#if NCURSES_XNAMES
NCURSES_EXPORT_VAR(bool) _nc_user_definable = TRUE;

NCURSES_EXPORT(int)
use_extended_names(bool flag)
{
    int oldflag = _nc_user_definable;

    T((T_CALLED("use_extended_names(%d)"), flag));
    _nc_user_definable = flag;
    returnBool(oldflag);
}
#endif