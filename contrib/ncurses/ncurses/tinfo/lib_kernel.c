
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
 *     and: Thomas E. Dickey 2002                                           *
 ****************************************************************************/

/*
 *	lib_kernel.c
 *
 *	Misc. low-level routines:
 *		erasechar()
 *		killchar()
 *		flushinp()
 *
 * The baudrate() and delay_output() functions could logically live here,
 * but are in other modules to reduce the static-link size of programs
 * that use only these facilities.
 */

#include <curses.priv.h>
#include <term.h>		/* cur_term */

MODULE_ID("$Id: lib_kernel.c,v 1.24 2004/05/08 17:11:21 tom Exp $")

static int
_nc_vdisable(void)
{
    int value = -1;
#if defined(_POSIX_VDISABLE) && HAVE_UNISTD_H
    value = _POSIX_VDISABLE;
#endif
#if defined(_PC_VDISABLE)
    if (value == -1) {
	value = fpathconf(0, _PC_VDISABLE);
	if (value == -1) {
	    value = 0377;
	}
    }
#elif defined(VDISABLE)
    if (value == -1)
	value = VDISABLE;
#endif
    return value;
}

/*
 *	erasechar()
 *
 *	Return erase character as given in cur_term->Ottyb.
 *
 */

NCURSES_EXPORT(char)
erasechar(void)
{
    int result = ERR;
    T((T_CALLED("erasechar()")));

    if (cur_term != 0) {
#ifdef TERMIOS
	result = cur_term->Ottyb.c_cc[VERASE];
	if (result == _nc_vdisable())
	    result = ERR;
#else
	result = cur_term->Ottyb.sg_erase;
#endif
    }
    returnCode(result);
}

/*
 *	killchar()
 *
 *	Return kill character as given in cur_term->Ottyb.
 *
 */

NCURSES_EXPORT(char)
killchar(void)
{
    int result = ERR;
    T((T_CALLED("killchar()")));

    if (cur_term != 0) {
#ifdef TERMIOS
	result = cur_term->Ottyb.c_cc[VKILL];
	if (result == _nc_vdisable())
	    result = ERR;
#else
	result = cur_term->Ottyb.sg_kill;
#endif
    }
    returnCode(result);
}

/*
 *	flushinp()
 *
 *	Flush any input on cur_term->Filedes
 *
 */

NCURSES_EXPORT(int)
flushinp(void)
{
    T((T_CALLED("flushinp()")));

    if (cur_term != 0) {
#ifdef TERMIOS
	tcflush(cur_term->Filedes, TCIFLUSH);
#else
	errno = 0;
	do {
	    ioctl(cur_term->Filedes, TIOCFLUSH, 0);
	} while
	    (errno == EINTR);
#endif
	if (SP) {
	    SP->_fifohead = -1;
	    SP->_fifotail = 0;
	    SP->_fifopeek = 0;
	}
	returnCode(OK);
    }
    returnCode(ERR);
}