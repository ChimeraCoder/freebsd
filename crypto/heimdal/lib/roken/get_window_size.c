
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

#include <config.h>

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if 0 /* Where were those needed? /confused */
#ifdef HAVE_SYS_PROC_H
#include <sys/proc.h>
#endif

#ifdef HAVE_SYS_TTY_H
#include <sys/tty.h>
#endif
#endif

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#include "roken.h"

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
get_window_size(int fd, int *lines, int *columns)
{
    char *s;

#if defined(TIOCGWINSZ)
    {
	struct winsize ws;
        int ret;
	ret = ioctl(fd, TIOCGWINSZ, &ws);
	if (ret != -1) {
	    if (lines)
		*lines = ws.ws_row;
	    if (columns)
		*columns = ws.ws_col;
	    return 0;
	}
    }
#elif defined(TIOCGSIZE)
    {
	struct ttysize ts;
        int ret;
	ret = ioctl(fd, TIOCGSIZE, &ts);
	if (ret != -1) {
	    if (lines)
		*lines = ts.ws_lines;
	    if (columns)
		*columns = ts.ts_cols;
	    return 0;
 	}
    }
#elif defined(HAVE__SCRSIZE)
    {
	int dst[2];

 	_scrsize(dst);
	if (lines)
	    *lines = dst[1];
	if (columns)
	    *columns = dst[0];
	return 0;
    }
#elif defined(_WIN32)
    {
        intptr_t fh = 0;
        CONSOLE_SCREEN_BUFFER_INFO sb_info;

        fh = _get_osfhandle(fd);
        if (fh != (intptr_t) INVALID_HANDLE_VALUE &&
            GetConsoleScreenBufferInfo((HANDLE) fh, &sb_info)) {
            if (lines)
                *lines = 1 + sb_info.srWindow.Bottom - sb_info.srWindow.Top;
            if (columns)
                *columns = 1 + sb_info.srWindow.Right - sb_info.srWindow.Left;

            return 0;
        }
    }
#endif
    if (columns) {
    	if ((s = getenv("COLUMNS")))
	    *columns = atoi(s);
	else
	    return -1;
    }
    if (lines) {
	if ((s = getenv("LINES")))
	    *lines = atoi(s);
	else
	    return -1;
    }
    return 0;
}