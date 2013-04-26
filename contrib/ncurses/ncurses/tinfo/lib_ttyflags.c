
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

/*
 *		def_prog_mode()
 *		def_shell_mode()
 *		reset_prog_mode()
 *		reset_shell_mode()
 *		savetty()
 *		resetty()
 */

#include <curses.priv.h>
#include <term.h>		/* cur_term */

MODULE_ID("$Id: lib_ttyflags.c,v 1.18 2008/08/03 22:10:44 tom Exp $")

NCURSES_EXPORT(int)
_nc_get_tty_mode(TTY * buf)
{
    int result = OK;

    if (buf == 0) {
	result = ERR;
    } else {
	if (cur_term == 0) {
	    result = ERR;
	} else {
	    for (;;) {
		if (GET_TTY(cur_term->Filedes, buf) != 0) {
		    if (errno == EINTR)
			continue;
		    result = ERR;
		}
		break;
	    }
	}

	if (result == ERR)
	    memset(buf, 0, sizeof(*buf));

	TR(TRACE_BITS, ("_nc_get_tty_mode(%d): %s",
			cur_term ? cur_term->Filedes : -1,
			_nc_trace_ttymode(buf)));
    }
    return (result);
}

NCURSES_EXPORT(int)
_nc_set_tty_mode(TTY * buf)
{
    int result = OK;

    if (buf == 0) {
	result = ERR;
    } else {
	if (cur_term == 0) {
	    result = ERR;
	} else {
	    for (;;) {
		if (SET_TTY(cur_term->Filedes, buf) != 0) {
		    if (errno == EINTR)
			continue;
		    if ((errno == ENOTTY) && (SP != 0))
			SP->_notty = TRUE;
		    result = ERR;
		}
		break;
	    }
	}
	TR(TRACE_BITS, ("_nc_set_tty_mode(%d): %s",
			cur_term ? cur_term->Filedes : -1,
			_nc_trace_ttymode(buf)));
    }
    return (result);
}

NCURSES_EXPORT(int)
def_shell_mode(void)
{
    int rc = ERR;

    T((T_CALLED("def_shell_mode()")));

    if (cur_term != 0) {
	/*
	 * If XTABS was on, remove the tab and backtab capabilities.
	 */
	if (_nc_get_tty_mode(&cur_term->Ottyb) == OK) {
#ifdef TERMIOS
	    if (cur_term->Ottyb.c_oflag & OFLAGS_TABS)
		tab = back_tab = NULL;
#else
	    if (cur_term->Ottyb.sg_flags & XTABS)
		tab = back_tab = NULL;
#endif
	    rc = OK;
	}
    }
    returnCode(rc);
}

NCURSES_EXPORT(int)
def_prog_mode(void)
{
    int rc = ERR;

    T((T_CALLED("def_prog_mode()")));

    if (cur_term != 0) {
	/*
	 * Turn off the XTABS bit in the tty structure if it was on.
	 */
	if (_nc_get_tty_mode(&cur_term->Nttyb) == OK) {
#ifdef TERMIOS
	    cur_term->Nttyb.c_oflag &= ~OFLAGS_TABS;
#else
	    cur_term->Nttyb.sg_flags &= ~XTABS;
#endif
	    rc = OK;
	}
    }
    returnCode(rc);
}

NCURSES_EXPORT(int)
reset_prog_mode(void)
{
    T((T_CALLED("reset_prog_mode()")));

    if (cur_term != 0) {
	if (_nc_set_tty_mode(&cur_term->Nttyb) == OK) {
	    if (SP) {
		if (SP->_keypad_on)
		    _nc_keypad(SP, TRUE);
		NC_BUFFERED(TRUE);
	    }
	    returnCode(OK);
	}
    }
    returnCode(ERR);
}

NCURSES_EXPORT(int)
reset_shell_mode(void)
{
    T((T_CALLED("reset_shell_mode()")));

    if (cur_term != 0) {
	if (SP) {
	    _nc_keypad(SP, FALSE);
	    _nc_flush();
	    NC_BUFFERED(FALSE);
	}
	returnCode(_nc_set_tty_mode(&cur_term->Ottyb));
    }
    returnCode(ERR);
}

static TTY *
saved_tty(void)
{
    TTY *result = 0;

    if (SP != 0) {
	result = &(SP->_saved_tty);
    } else {
	if (_nc_prescreen.saved_tty == 0) {
	    _nc_prescreen.saved_tty = typeCalloc(TTY, 1);
	}
	result = _nc_prescreen.saved_tty;
    }
    return result;
}

/*
**	savetty()  and  resetty()
**
*/

NCURSES_EXPORT(int)
savetty(void)
{
    T((T_CALLED("savetty()")));

    returnCode(_nc_get_tty_mode(saved_tty()));
}

NCURSES_EXPORT(int)
resetty(void)
{
    T((T_CALLED("resetty()")));

    returnCode(_nc_set_tty_mode(saved_tty()));
}