
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
 * Terminfo-only terminal setup routines:
 *
 *		int restartterm(const char *, int, int *)
 */

#include <curses.priv.h>

#if SVR4_TERMIO && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#endif

#include <term.h>		/* lines, columns, cur_term */

MODULE_ID("$Id: lib_restart.c,v 1.10 2008/06/21 17:31:22 tom Exp $")

NCURSES_EXPORT(int)
restartterm(NCURSES_CONST char *termp, int filenum, int *errret)
{
    int result;

    T((T_CALLED("restartterm(%s,%d,%p)"), termp, filenum, errret));

    if (setupterm(termp, filenum, errret) != OK) {
	result = ERR;
    } else if (SP != 0) {
	int saveecho = SP->_echo;
	int savecbreak = SP->_cbreak;
	int saveraw = SP->_raw;
	int savenl = SP->_nl;

	if (saveecho)
	    echo();
	else
	    noecho();

	if (savecbreak) {
	    cbreak();
	    noraw();
	} else if (saveraw) {
	    nocbreak();
	    raw();
	} else {
	    nocbreak();
	    noraw();
	}
	if (savenl)
	    nl();
	else
	    nonl();

	reset_prog_mode();

#if USE_SIZECHANGE
	_nc_update_screensize(SP);
#endif

	result = OK;
    } else {
	result = ERR;
    }
    returnCode(result);
}