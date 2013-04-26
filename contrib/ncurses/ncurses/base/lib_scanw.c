
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
**	lib_scanw.c
**
**	The routines scanw(), wscanw() and friends.
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_scanw.c,v 1.11 2001/06/30 23:39:41 tom Exp $")

NCURSES_EXPORT(int)
vwscanw(WINDOW *win, NCURSES_CONST char *fmt, va_list argp)
{
    char buf[BUFSIZ];

    if (wgetnstr(win, buf, sizeof(buf) - 1) == ERR)
	return (ERR);

    return (vsscanf(buf, fmt, argp));
}

NCURSES_EXPORT(int)
scanw(NCURSES_CONST char *fmt,...)
{
    int code;
    va_list ap;

    T(("scanw(\"%s\",...) called", fmt));

    va_start(ap, fmt);
    code = vwscanw(stdscr, fmt, ap);
    va_end(ap);
    return (code);
}

NCURSES_EXPORT(int)
wscanw(WINDOW *win, NCURSES_CONST char *fmt,...)
{
    int code;
    va_list ap;

    T(("wscanw(%p,\"%s\",...) called", win, fmt));

    va_start(ap, fmt);
    code = vwscanw(win, fmt, ap);
    va_end(ap);
    return (code);
}

NCURSES_EXPORT(int)
mvscanw(int y, int x, NCURSES_CONST char *fmt,...)
{
    int code;
    va_list ap;

    va_start(ap, fmt);
    code = (move(y, x) == OK) ? vwscanw(stdscr, fmt, ap) : ERR;
    va_end(ap);
    return (code);
}

NCURSES_EXPORT(int)
mvwscanw(WINDOW *win, int y, int x, NCURSES_CONST char *fmt,...)
{
    int code;
    va_list ap;

    va_start(ap, fmt);
    code = (wmove(win, y, x) == OK) ? vwscanw(win, fmt, ap) : ERR;
    va_end(ap);
    return (code);
}