
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

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

#ifndef lint
static const char sccsid[] = "@(#)display.c	8.1 (Berkeley) 6/6/93";
#endif

/*
 * The window 'manager', initializes curses and handles the actual
 * displaying of text
 */
#include <ctype.h>
#include <unistd.h>

#include "talk.h"

xwin_t	my_win;
xwin_t	his_win;
WINDOW	*line_win;

int	curses_initialized = 0;

/*
 * max HAS to be a function, it is called with
 * an argument of the form --foo at least once.
 */
int
max(int a, int b)
{

	return (a > b ? a : b);
}

/*
 * Display some text on somebody's window, processing some control
 * characters while we are at it.
 */
void
display(xwin_t *win, char *text, int size)
{
	int i;
	char cch;

	for (i = 0; i < size; i++) {
		if (*text == '\n' || *text == '\r') {
			waddch(win->x_win, '\n');
			getyx(win->x_win, win->x_line, win->x_col);
			text++;
			continue;
		}
		if (*text == 004 && win == &my_win) {
			/* control-D clears the screen */
			werase(my_win.x_win);
			getyx(my_win.x_win, my_win.x_line, my_win.x_col);
			wrefresh(my_win.x_win);
			werase(his_win.x_win);
			getyx(his_win.x_win, his_win.x_line, his_win.x_col);
			wrefresh(his_win.x_win);
			text++;
			continue;
		}

		/* erase character */
		if (   *text == win->cerase
		    || *text == 010     /* BS */
		    || *text == 0177    /* DEL */
		   ) {
			wmove(win->x_win, win->x_line, max(--win->x_col, 0));
			getyx(win->x_win, win->x_line, win->x_col);
			waddch(win->x_win, ' ');
			wmove(win->x_win, win->x_line, win->x_col);
			getyx(win->x_win, win->x_line, win->x_col);
			text++;
			continue;
		}
		/*
		 * On word erase search backwards until we find
		 * the beginning of a word or the beginning of
		 * the line.
		 */
		if (   *text == win->werase
		    || *text == 027     /* ^W */
		   ) {
			int endcol, xcol, ii, c;

			endcol = win->x_col;
			xcol = endcol - 1;
			while (xcol >= 0) {
				c = readwin(win->x_win, win->x_line, xcol);
				if (c != ' ')
					break;
				xcol--;
			}
			while (xcol >= 0) {
				c = readwin(win->x_win, win->x_line, xcol);
				if (c == ' ')
					break;
				xcol--;
			}
			wmove(win->x_win, win->x_line, xcol + 1);
			for (ii = xcol + 1; ii < endcol; ii++)
				waddch(win->x_win, ' ');
			wmove(win->x_win, win->x_line, xcol + 1);
			getyx(win->x_win, win->x_line, win->x_col);
			text++;
			continue;
		}
		/* line kill */
		if (   *text == win->kill
		    || *text == 025     /* ^U */
		   ) {
			wmove(win->x_win, win->x_line, 0);
			wclrtoeol(win->x_win);
			getyx(win->x_win, win->x_line, win->x_col);
			text++;
			continue;
		}
		if (*text == '\f') {
			if (win == &my_win)
				wrefresh(curscr);
			text++;
			continue;
		}
		if (*text == '\7') {
			write(STDOUT_FILENO, text, 1);
			text++;
			continue;
		}
		if (!isprint((unsigned char)*text) && *text != '\t') {
			waddch(win->x_win, '^');
			getyx(win->x_win, win->x_line, win->x_col);
			cch = (*text & 63) + 64;
			waddch(win->x_win, cch);
		} else
			waddch(win->x_win, (unsigned char)*text);
		getyx(win->x_win, win->x_line, win->x_col);
		text++;
	}
	wrefresh(win->x_win);
}

/*
 * Read the character at the indicated position in win
 */
int
readwin(WINDOW *win, int line, int col)
{
	int oldline, oldcol;
	int c;

	getyx(win, oldline, oldcol);
	wmove(win, line, col);
	c = winch(win);
	wmove(win, oldline, oldcol);
	return (c);
}