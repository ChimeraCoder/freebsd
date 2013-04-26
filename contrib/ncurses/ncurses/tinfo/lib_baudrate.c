
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
 *	lib_baudrate.c
 *
 */

#include <curses.priv.h>
#include <term.h>		/* cur_term, pad_char */
#include <termcap.h>		/* ospeed */
#if defined(__FreeBSD__)
#include <sys/param.h>
#endif

/*
 * These systems use similar header files, which define B1200 as 1200, etc.,
 * but can be overridden by defining USE_OLD_TTY so B1200 is 9, which makes all
 * of the indices up to B115200 fit nicely in a 'short', allowing us to retain
 * ospeed's type for compatibility.
 */
#if (defined(__FreeBSD__) && (__FreeBSD_version < 700000)) || defined(__NetBSD__) || defined(__OpenBSD__)
#undef B0
#undef B50
#undef B75
#undef B110
#undef B134
#undef B150
#undef B200
#undef B300
#undef B600
#undef B1200
#undef B1800
#undef B2400
#undef B4800
#undef B9600
#undef B19200
#undef EXTA
#undef B38400
#undef EXTB
#undef B57600
#undef B115200
#undef B230400
#undef B460800
#undef B921600
#define USE_OLD_TTY
#include <sys/ttydev.h>
#else
#undef USE_OLD_TTY
#endif /* USE_OLD_TTY */

MODULE_ID("$Id: lib_baudrate.c,v 1.27 2008/06/28 15:19:24 tom Exp $")

/*
 *	int
 *	baudrate()
 *
 *	Returns the current terminal's baud rate.
 *
 */

struct speed {
    int s;			/* value for 'ospeed' is an index */
    int sp;			/* the actual speed */
};

static struct speed const speeds[] =
{
    {B0, 0},
    {B50, 50},
    {B75, 75},
    {B110, 110},
    {B134, 134},
    {B150, 150},
    {B200, 200},
    {B300, 300},
    {B600, 600},
    {B1200, 1200},
    {B1800, 1800},
    {B2400, 2400},
    {B4800, 4800},
    {B9600, 9600},
#ifdef B19200
    {B19200, 19200},
#else
#ifdef EXTA
    {EXTA, 19200},
#endif
#endif
#ifdef B38400
    {B38400, 38400},
#else
#ifdef EXTB
    {EXTB, 38400},
#endif
#endif
#ifdef B57600
    {B57600, 57600},
#endif
#ifdef B115200
    {B115200, 115200},
#endif
#ifdef B230400
    {B230400, 230400},
#endif
#ifdef B460800
    {B460800, 460800},
#endif
#ifdef B921600
    {B921600, 921600},
#endif
};

NCURSES_EXPORT(int)
_nc_baudrate(int OSpeed)
{
#if !USE_REENTRANT
    static int last_OSpeed;
    static int last_baudrate;
#endif

    int result = ERR;
    unsigned i;

#if !USE_REENTRANT
    if (OSpeed == last_OSpeed) {
	result = last_baudrate;
    }
#endif
    if (result == ERR) {
	if (OSpeed >= 0) {
	    for (i = 0; i < SIZEOF(speeds); i++) {
		if (speeds[i].s == OSpeed) {
		    result = speeds[i].sp;
		    break;
		}
	    }
	}
#if !USE_REENTRANT
	if (OSpeed == last_OSpeed) {
	    last_OSpeed = OSpeed;
	    last_baudrate = result;
	}
#endif
    }
    return (result);
}

NCURSES_EXPORT(int)
_nc_ospeed(int BaudRate)
{
    int result = 1;
    unsigned i;

    if (BaudRate >= 0) {
	for (i = 0; i < SIZEOF(speeds); i++) {
	    if (speeds[i].sp == BaudRate) {
		result = speeds[i].s;
		break;
	    }
	}
    }
    return (result);
}

NCURSES_EXPORT(int)
baudrate(void)
{
    int result;

    T((T_CALLED("baudrate()")));

    /*
     * In debugging, allow the environment symbol to override when we're
     * redirecting to a file, so we can construct repeatable test-cases
     * that take into account costs that depend on baudrate.
     */
#ifdef TRACE
    if (!isatty(fileno(SP ? SP->_ofp : stdout))
	&& getenv("BAUDRATE") != 0) {
	int ret;
	if ((ret = _nc_getenv_num("BAUDRATE")) <= 0)
	    ret = 9600;
	ospeed = _nc_ospeed(ret);
	returnCode(ret);
    }
#endif

    if (cur_term != 0) {
#ifdef USE_OLD_TTY
	result = cfgetospeed(&cur_term->Nttyb);
	ospeed = _nc_ospeed(result);
#else /* !USE_OLD_TTY */
#ifdef TERMIOS
	ospeed = cfgetospeed(&cur_term->Nttyb);
#else
	ospeed = cur_term->Nttyb.sg_ospeed;
#endif
	result = _nc_baudrate(ospeed);
#endif
	cur_term->_baudrate = result;
    } else {
	result = ERR;
    }

    returnCode(result);
}