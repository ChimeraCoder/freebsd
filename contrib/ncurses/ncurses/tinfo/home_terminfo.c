
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

/*
 *	home_terminfo.c -- return the $HOME/.terminfo string, expanded
 */

#include <curses.priv.h>
#include <tic.h>

MODULE_ID("$Id: home_terminfo.c,v 1.11 2008/08/03 23:43:11 tom Exp $")

/* ncurses extension...fall back on user's private directory */

#define MyBuffer _nc_globals.home_terminfo

NCURSES_EXPORT(char *)
_nc_home_terminfo(void)
{
    char *result = 0;
#if USE_HOME_TERMINFO
    char *home;

    if (use_terminfo_vars()) {
	if (MyBuffer == 0) {
	    if ((home = getenv("HOME")) != 0) {
		unsigned want = (strlen(home) + sizeof(PRIVATE_INFO));
		MyBuffer = typeMalloc(char, want);
		if (MyBuffer == 0)
		    _nc_err_abort(MSG_NO_MEMORY);
		(void) sprintf(MyBuffer, PRIVATE_INFO, home);
	    }
	}
	result = MyBuffer;
    }
#endif
    return result;
}