
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1997                        *
 ****************************************************************************/

/*
**	lib_printw.c
**
**	The routines printw(), wprintw() and friends.
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_printw.c,v 1.18 2006/12/17 19:21:39 tom Exp $")

NCURSES_EXPORT(int)
printw(const char *fmt,...)
{
    va_list argp;
    int code;

#ifdef TRACE
    va_start(argp, fmt);
    T((T_CALLED("printw(%s%s)"),
       _nc_visbuf(fmt), _nc_varargs(fmt, argp)));
    va_end(argp);
#endif

    va_start(argp, fmt);
    code = vwprintw(stdscr, fmt, argp);
    va_end(argp);

    returnCode(code);
}

NCURSES_EXPORT(int)
wprintw(WINDOW *win, const char *fmt,...)
{
    va_list argp;
    int code;

#ifdef TRACE
    va_start(argp, fmt);
    T((T_CALLED("wprintw(%p,%s%s)"),
       win, _nc_visbuf(fmt), _nc_varargs(fmt, argp)));
    va_end(argp);
#endif

    va_start(argp, fmt);
    code = vwprintw(win, fmt, argp);
    va_end(argp);

    returnCode(code);
}

NCURSES_EXPORT(int)
mvprintw(int y, int x, const char *fmt,...)
{
    va_list argp;
    int code;

#ifdef TRACE
    va_start(argp, fmt);
    T((T_CALLED("mvprintw(%d,%d,%s%s)"),
       y, x, _nc_visbuf(fmt), _nc_varargs(fmt, argp)));
    va_end(argp);
#endif

    if ((code = move(y, x)) != ERR) {
	va_start(argp, fmt);
	code = vwprintw(stdscr, fmt, argp);
	va_end(argp);
    }
    returnCode(code);
}

NCURSES_EXPORT(int)
mvwprintw(WINDOW *win, int y, int x, const char *fmt,...)
{
    va_list argp;
    int code;

#ifdef TRACE
    va_start(argp, fmt);
    T((T_CALLED("mvwprintw(%d,%d,%p,%s%s)"),
       y, x, win, _nc_visbuf(fmt), _nc_varargs(fmt, argp)));
    va_end(argp);
#endif

    if ((code = wmove(win, y, x)) != ERR) {
	va_start(argp, fmt);
	code = vwprintw(win, fmt, argp);
	va_end(argp);
    }
    returnCode(code);
}

NCURSES_EXPORT(int)
vwprintw(WINDOW *win, const char *fmt, va_list argp)
{
    char *buf;
    int code = ERR;

    T((T_CALLED("vwprintw(%p,%s,va_list)"), win, _nc_visbuf(fmt)));

    if ((buf = _nc_printf_string(fmt, argp)) != 0) {
	code = waddstr(win, buf);
    }
    returnCode(code);
}