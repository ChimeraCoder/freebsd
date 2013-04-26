
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1997                        *
 ****************************************************************************/
/*
 * Module that "owns" the 'cur_term' variable:
 *
 *	TERMINAL *set_curterm(TERMINAL *)
 *	int del_curterm(TERMINAL *)
 */

#include <curses.priv.h>
#include <term_entry.h>		/* TTY, cur_term */
#include <termcap.h>		/* ospeed */

MODULE_ID("$Id: lib_cur_term.c,v 1.18 2008/08/16 19:22:55 tom Exp $")

#undef CUR
#define CUR termp->type.

#if BROKEN_LINKER || USE_REENTRANT
NCURSES_EXPORT(TERMINAL *)
NCURSES_PUBLIC_VAR(cur_term) (void)
{
    return (SP != 0 && SP->_term != 0) ? SP->_term : _nc_prescreen._cur_term;
}
#else
NCURSES_EXPORT_VAR(TERMINAL *) cur_term = 0;
#endif

NCURSES_EXPORT(TERMINAL *)
set_curterm(TERMINAL * termp)
{
    TERMINAL *oldterm;

    T((T_CALLED("set_curterm(%p)"), termp));

    _nc_lock_global(curses);
    oldterm = cur_term;
    if (SP)
	SP->_term = termp;
#if BROKEN_LINKER || USE_REENTRANT
    _nc_prescreen._cur_term = termp;
#else
    cur_term = termp;
#endif
    if (termp != 0) {
	ospeed = _nc_ospeed(termp->_baudrate);
	if (termp->type.Strings) {
	    PC = (char) ((pad_char != NULL) ? pad_char[0] : 0);
	}
    }
    _nc_unlock_global(curses);

    T((T_RETURN("%p"), oldterm));
    return (oldterm);
}

NCURSES_EXPORT(int)
del_curterm(TERMINAL * termp)
{
    int rc = ERR;

    T((T_CALLED("del_curterm(%p)"), termp));

    _nc_lock_global(curses);
    if (termp != 0) {
	_nc_free_termtype(&(termp->type));
	FreeIfNeeded(termp->_termname);
	free(termp);
	if (termp == cur_term)
	    set_curterm(0);
	rc = OK;
    }
    _nc_unlock_global(curses);

    returnCode(rc);
}