
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

/*
 *	lib_napms.c
 *
 *	The routine napms.
 *
 *	(This file was originally written by Eric Raymond; however except for
 *	comments, none of the original code remains - T.Dickey).
 */

#include <curses.priv.h>

#if HAVE_NANOSLEEP
#include <time.h>
#if HAVE_SYS_TIME_H
#include <sys/time.h>		/* needed for MacOS X DP3 */
#endif
#endif

MODULE_ID("$Id: lib_napms.c,v 1.17 2008/05/03 21:34:13 tom Exp $")

NCURSES_EXPORT(int)
napms(int ms)
{
    T((T_CALLED("napms(%d)"), ms));

#if HAVE_NANOSLEEP
    {
	struct timespec request, remaining;
	request.tv_sec = ms / 1000;
	request.tv_nsec = (ms % 1000) * 1000000;
	while (nanosleep(&request, &remaining) == -1
	       && errno == EINTR) {
	    request = remaining;
	}
    }
#else
    _nc_timed_wait(0, 0, ms, (int *) 0 EVENTLIST_2nd(0));
#endif

    returnCode(OK);
}