
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

#include <curses.priv.h>

#include <term_entry.h>
#include <tic.h>

MODULE_ID("$Id: lib_ti.c,v 1.23 2003/05/24 21:10:28 tom Exp $")

NCURSES_EXPORT(int)
tigetflag(NCURSES_CONST char *str)
{
    unsigned i;

    T((T_CALLED("tigetflag(%s)"), str));

    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_boolean(i, tp) {
	    const char *capname = ExtBoolname(tp, i, boolnames);
	    if (!strcmp(str, capname)) {
		/* setupterm forces invalid booleans to false */
		returnCode(tp->Booleans[i]);
	    }
	}
    }

    returnCode(ABSENT_BOOLEAN);
}

NCURSES_EXPORT(int)
tigetnum(NCURSES_CONST char *str)
{
    unsigned i;

    T((T_CALLED("tigetnum(%s)"), str));

    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_number(i, tp) {
	    const char *capname = ExtNumname(tp, i, numnames);
	    if (!strcmp(str, capname)) {
		if (!VALID_NUMERIC(tp->Numbers[i]))
		    returnCode(ABSENT_NUMERIC);
		returnCode(tp->Numbers[i]);
	    }
	}
    }

    returnCode(CANCELLED_NUMERIC);	/* Solaris returns a -1 instead */
}

NCURSES_EXPORT(char *)
tigetstr(NCURSES_CONST char *str)
{
    unsigned i;

    T((T_CALLED("tigetstr(%s)"), str));

    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_string(i, tp) {
	    const char *capname = ExtStrname(tp, i, strnames);
	    if (!strcmp(str, capname)) {
		/* setupterm forces cancelled strings to null */
		returnPtr(tp->Strings[i]);
	    }
	}
    }

    returnPtr(CANCELLED_STRING);
}