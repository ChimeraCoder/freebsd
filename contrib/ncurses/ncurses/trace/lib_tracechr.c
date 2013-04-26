
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
 *	lib_tracechr.c - Tracing/Debugging routines
 */
#include <curses.priv.h>

#include <ctype.h>

MODULE_ID("$Id: lib_tracechr.c,v 1.19 2008/08/03 15:39:29 tom Exp $")

#ifdef TRACE

NCURSES_EXPORT(char *)
_nc_tracechar(SCREEN *sp, int ch)
{
    NCURSES_CONST char *name;
    char *MyBuffer = ((sp != 0)
		      ? sp->tracechr_buf
		      : _nc_globals.tracechr_buf);

    if (ch > KEY_MIN || ch < 0) {
	name = _nc_keyname(sp, ch);
	if (name == 0 || *name == '\0')
	    name = "NULL";
	(void) sprintf(MyBuffer, "'%.30s' = %#03o", name, ch);
    } else if (!is8bits(ch) || !isprint(UChar(ch))) {
	/*
	 * workaround for glibc bug:
	 * sprintf changes the result from unctrl() to an empty string if it
	 * does not correspond to a valid multibyte sequence.
	 */
	(void) sprintf(MyBuffer, "%#03o", ch);
    } else {
	name = _nc_unctrl(sp, (chtype) ch);
	if (name == 0 || *name == 0)
	    name = "null";	/* shouldn't happen */
	(void) sprintf(MyBuffer, "'%.30s' = %#03o", name, ch);
    }
    return (MyBuffer);
}

NCURSES_EXPORT(char *)
_tracechar(int ch)
{
    return _nc_tracechar(SP, ch);
}
#else
EMPTY_MODULE(_nc_lib_tracechr)
#endif