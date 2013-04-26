
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
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
**	lib_ungetch.c
**
**	The routine ungetch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_ungetch.c,v 1.11 2008/05/31 16:44:54 tom Exp $")

#include <fifo_defs.h>

#ifdef TRACE
NCURSES_EXPORT(void)
_nc_fifo_dump(SCREEN *sp)
{
    int i;
    T(("head = %d, tail = %d, peek = %d", head, tail, peek));
    for (i = 0; i < 10; i++)
	T(("char %d = %s", i, _nc_tracechar(sp, sp->_fifo[i])));
}
#endif /* TRACE */

NCURSES_EXPORT(int)
_nc_ungetch(SCREEN *sp, int ch)
{
    int rc = ERR;

    if (tail != -1) {
	if (head == -1) {
	    head = 0;
	    t_inc();
	    peek = tail;	/* no raw keys */
	} else
	    h_dec();

	sp->_fifo[head] = ch;
	T(("ungetch %s ok", _nc_tracechar(sp, ch)));
#ifdef TRACE
	if (USE_TRACEF(TRACE_IEVENT)) {
	    _nc_fifo_dump(sp);
	    _nc_unlock_global(tracef);
	}
#endif
	rc = OK;
    }
    return rc;
}

NCURSES_EXPORT(int)
ungetch(int ch)
{
    T((T_CALLED("ungetch(%s)"), _nc_tracechar(SP, ch)));
    returnCode(_nc_ungetch(SP, ch));
}