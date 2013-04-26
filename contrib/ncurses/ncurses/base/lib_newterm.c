
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
**	lib_newterm.c
**
**	The newterm() function.
**
*/

#include <curses.priv.h>

#if SVR4_TERMIO && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#endif

#include <term.h>		/* clear_screen, cup & friends, cur_term */
#include <tic.h>

MODULE_ID("$Id: lib_newterm.c,v 1.73 2008/08/16 21:20:48 Werner.Fink Exp $")

#ifndef ONLCR			/* Allows compilation under the QNX 4.2 OS */
#define ONLCR 0
#endif

/*
 * SVr4/XSI Curses specify that hardware echo is turned off in initscr, and not
 * restored during the curses session.  The library simulates echo in software.
 * (The behavior is unspecified if the application enables hardware echo).
 *
 * The newterm function also initializes terminal settings, and since initscr
 * is supposed to behave as if it calls newterm, we do it here.
 */
static NCURSES_INLINE int
_nc_initscr(void)
{
    int result = ERR;

    /* for extended XPG4 conformance requires cbreak() at this point */
    /* (SVr4 curses does this anyway) */
    if (cbreak() == OK) {
	TTY buf;

	buf = cur_term->Nttyb;
#ifdef TERMIOS
	buf.c_lflag &= ~(ECHO | ECHONL);
	buf.c_iflag &= ~(ICRNL | INLCR | IGNCR);
	buf.c_oflag &= ~(ONLCR);
#elif HAVE_SGTTY_H
	buf.sg_flags &= ~(ECHO | CRMOD);
#else
	memset(&buf, 0, sizeof(buf));
#endif
	if ((result = _nc_set_tty_mode(&buf)) == OK)
	    cur_term->Nttyb = buf;
    }
    return result;
}

/*
 * filter() has to be called before either initscr() or newterm(), so there is
 * apparently no way to make this flag apply to some terminals and not others,
 * aside from possibly delaying a filter() call until some terminals have been
 * initialized.
 */
NCURSES_EXPORT(void)
filter(void)
{
    START_TRACE();
    T((T_CALLED("filter")));
    _nc_prescreen.filter_mode = TRUE;
    returnVoid;
}

#if NCURSES_EXT_FUNCS
/*
 * An extension, allowing the application to open a new screen without
 * requiring it to also be filtered.
 */
NCURSES_EXPORT(void)
nofilter(void)
{
    START_TRACE();
    T((T_CALLED("nofilter")));
    _nc_prescreen.filter_mode = FALSE;
    returnVoid;
}
#endif

NCURSES_EXPORT(SCREEN *)
newterm(NCURSES_CONST char *name, FILE *ofp, FILE *ifp)
{
    int value;
    int errret;
    SCREEN *current;
    SCREEN *result = 0;
    TERMINAL *its_term;

    START_TRACE();
    T((T_CALLED("newterm(\"%s\",%p,%p)"), name, ofp, ifp));

    _nc_init_pthreads();
    _nc_lock_global(curses);

    current = SP;
    its_term = (SP ? SP->_term : 0);

    /* this loads the capability entry, then sets LINES and COLS */
    if (setupterm(name, fileno(ofp), &errret) != ERR) {
	int slk_format = _nc_globals.slk_format;

	/*
	 * This actually allocates the screen structure, and saves the original
	 * terminal settings.
	 */
	_nc_set_screen(0);

	/* allow user to set maximum escape delay from the environment */
	if ((value = _nc_getenv_num("ESCDELAY")) >= 0) {
	    set_escdelay(value);
	}

	if (_nc_setupscreen(LINES,
			    COLS,
			    ofp,
			    _nc_prescreen.filter_mode,
			    slk_format) == ERR) {
	    _nc_set_screen(current);
	    result = 0;
	} else {
	    assert(SP != 0);
	    /*
	     * In setupterm() we did a set_curterm(), but it was before we set
	     * SP.  So the "current" screen's terminal pointer was overwritten
	     * with a different terminal.  Later, in _nc_setupscreen(), we set
	     * SP and the terminal pointer in the new screen.
	     *
	     * Restore the terminal-pointer for the pre-existing screen, if
	     * any.
	     */
	    if (current)
		current->_term = its_term;

	    /* if the terminal type has real soft labels, set those up */
	    if (slk_format && num_labels > 0 && SLK_STDFMT(slk_format))
		_nc_slk_initialize(stdscr, COLS);

	    SP->_ifd = fileno(ifp);
	    typeahead(fileno(ifp));
#ifdef TERMIOS
	    SP->_use_meta = ((cur_term->Ottyb.c_cflag & CSIZE) == CS8 &&
			     !(cur_term->Ottyb.c_iflag & ISTRIP));
#else
	    SP->_use_meta = FALSE;
#endif
	    SP->_endwin = FALSE;

	    /*
	     * Check whether we can optimize scrolling under dumb terminals in
	     * case we do not have any of these capabilities, scrolling
	     * optimization will be useless.
	     */
	    SP->_scrolling = ((scroll_forward && scroll_reverse) ||
			      ((parm_rindex ||
				parm_insert_line ||
				insert_line) &&
			       (parm_index ||
				parm_delete_line ||
				delete_line)));

	    baudrate();		/* sets a field in the SP structure */

	    SP->_keytry = 0;

	    /*
	     * Check for mismatched graphic-rendition capabilities.  Most SVr4
	     * terminfo trees contain entries that have rmul or rmso equated to
	     * sgr0 (Solaris curses copes with those entries).  We do this only
	     * for curses, since many termcap applications assume that
	     * smso/rmso and smul/rmul are paired, and will not function
	     * properly if we remove rmso or rmul.  Curses applications
	     * shouldn't be looking at this detail.
	     */
#define SGR0_TEST(mode) (mode != 0) && (exit_attribute_mode == 0 || strcmp(mode, exit_attribute_mode))
	    SP->_use_rmso = SGR0_TEST(exit_standout_mode);
	    SP->_use_rmul = SGR0_TEST(exit_underline_mode);

	    /* compute movement costs so we can do better move optimization */
	    _nc_mvcur_init();

	    /* initialize terminal to a sane state */
	    _nc_screen_init();

	    /* Initialize the terminal line settings. */
	    _nc_initscr();

	    _nc_signal_handler(TRUE);

	    result = SP;
	}
    }
    _nc_unlock_global(curses);
    returnSP(result);
}