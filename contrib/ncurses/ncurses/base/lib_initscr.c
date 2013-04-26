
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
 *     and: Thomas E. Dickey                        1996-2003               *
 ****************************************************************************/

/*
**	lib_initscr.c
**
**	The routines initscr(), and termname().
**
*/

#include <curses.priv.h>

#if HAVE_SYS_TERMIO_H
#include <sys/termio.h>		/* needed for ISC */
#endif

MODULE_ID("$Id: lib_initscr.c,v 1.38 2008/08/16 21:20:48 Werner.Fink Exp $")

NCURSES_EXPORT(WINDOW *)
initscr(void)
{
    WINDOW *result;

    NCURSES_CONST char *name;

    START_TRACE();
    T((T_CALLED("initscr()")));

    _nc_init_pthreads();
    _nc_lock_global(curses);

    /* Portable applications must not call initscr() more than once */
    if (!_nc_globals.init_screen) {
	_nc_globals.init_screen = TRUE;

	if ((name = getenv("TERM")) == 0
	    || *name == '\0')
	    name = "unknown";
#ifdef __CYGWIN__
	/*
	 * 2002/9/21
	 * Work around a bug in Cygwin.  Full-screen subprocesses run from
	 * bash, in turn spawned from another full-screen process, will dump
	 * core when attempting to write to stdout.  Opening /dev/tty
	 * explicitly seems to fix the problem.
	 */
	if (isatty(fileno(stdout))) {
	    FILE *fp = fopen("/dev/tty", "w");
	    if (fp != 0 && isatty(fileno(fp))) {
		fclose(stdout);
		dup2(fileno(fp), STDOUT_FILENO);
		stdout = fdopen(STDOUT_FILENO, "w");
	    }
	}
#endif
	if (newterm(name, stdout, stdin) == 0) {
	    fprintf(stderr, "Error opening terminal: %s.\n", name);
	    exit(EXIT_FAILURE);
	}

	/* def_shell_mode - done in newterm/_nc_setupscreen */
	def_prog_mode();
    }
    result = stdscr;
    _nc_unlock_global(curses);

    returnWin(result);
}