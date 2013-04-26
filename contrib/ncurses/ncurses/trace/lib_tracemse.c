
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
 *	lib_tracemse.c - Tracing/Debugging routines (mouse events)
 */

#include <curses.priv.h>

MODULE_ID("$Id: lib_tracemse.c,v 1.15 2008/08/03 15:39:29 tom Exp $")

#ifdef TRACE

#define my_buffer sp->tracemse_buf

NCURSES_EXPORT(char *)
_nc_tracemouse(SCREEN *sp, MEVENT const *ep)
{
    (void) sprintf(my_buffer, TRACEMSE_FMT,
		   ep->id,
		   ep->x,
		   ep->y,
		   ep->z,
		   (unsigned long) ep->bstate);

#define SHOW(m, s) if ((ep->bstate & m) == m) strcat(strcat(my_buffer, s), ", ")

    SHOW(BUTTON1_RELEASED, "release-1");
    SHOW(BUTTON1_PRESSED, "press-1");
    SHOW(BUTTON1_CLICKED, "click-1");
    SHOW(BUTTON1_DOUBLE_CLICKED, "doubleclick-1");
    SHOW(BUTTON1_TRIPLE_CLICKED, "tripleclick-1");
#if NCURSES_MOUSE_VERSION == 1
    SHOW(BUTTON1_RESERVED_EVENT, "reserved-1");
#endif

    SHOW(BUTTON2_RELEASED, "release-2");
    SHOW(BUTTON2_PRESSED, "press-2");
    SHOW(BUTTON2_CLICKED, "click-2");
    SHOW(BUTTON2_DOUBLE_CLICKED, "doubleclick-2");
    SHOW(BUTTON2_TRIPLE_CLICKED, "tripleclick-2");
#if NCURSES_MOUSE_VERSION == 1
    SHOW(BUTTON2_RESERVED_EVENT, "reserved-2");
#endif

    SHOW(BUTTON3_RELEASED, "release-3");
    SHOW(BUTTON3_PRESSED, "press-3");
    SHOW(BUTTON3_CLICKED, "click-3");
    SHOW(BUTTON3_DOUBLE_CLICKED, "doubleclick-3");
    SHOW(BUTTON3_TRIPLE_CLICKED, "tripleclick-3");
#if NCURSES_MOUSE_VERSION == 1
    SHOW(BUTTON3_RESERVED_EVENT, "reserved-3");
#endif

    SHOW(BUTTON4_RELEASED, "release-4");
    SHOW(BUTTON4_PRESSED, "press-4");
    SHOW(BUTTON4_CLICKED, "click-4");
    SHOW(BUTTON4_DOUBLE_CLICKED, "doubleclick-4");
    SHOW(BUTTON4_TRIPLE_CLICKED, "tripleclick-4");
#if NCURSES_MOUSE_VERSION == 1
    SHOW(BUTTON4_RESERVED_EVENT, "reserved-4");
#endif

#if NCURSES_MOUSE_VERSION == 2
    SHOW(BUTTON5_RELEASED, "release-5");
    SHOW(BUTTON5_PRESSED, "press-5");
    SHOW(BUTTON5_CLICKED, "click-5");
    SHOW(BUTTON5_DOUBLE_CLICKED, "doubleclick-5");
    SHOW(BUTTON5_TRIPLE_CLICKED, "tripleclick-5");
#endif

    SHOW(BUTTON_CTRL, "ctrl");
    SHOW(BUTTON_SHIFT, "shift");
    SHOW(BUTTON_ALT, "alt");
    SHOW(ALL_MOUSE_EVENTS, "all-events");
    SHOW(REPORT_MOUSE_POSITION, "position");

#undef SHOW

    if (my_buffer[strlen(my_buffer) - 1] == ' ')
	my_buffer[strlen(my_buffer) - 2] = '\0';
    (void) strcat(my_buffer, "}");
    return (my_buffer);
}

NCURSES_EXPORT(char *)
_tracemouse(MEVENT const *ep)
{
    return _nc_tracemouse(SP, ep);
}

#else /* !TRACE */
EMPTY_MODULE(_nc_lib_tracemouse)
#endif