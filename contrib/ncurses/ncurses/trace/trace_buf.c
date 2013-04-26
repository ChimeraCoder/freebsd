
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
 *  Author: Thomas E. Dickey                 1997-on                        *
 ****************************************************************************/
/*
 *	trace_buf.c - Tracing/Debugging buffers (attributes)
 */

#include <curses.priv.h>

MODULE_ID("$Id: trace_buf.c,v 1.14 2008/08/03 15:13:56 tom Exp $")

#define MyList _nc_globals.tracebuf_ptr
#define MySize _nc_globals.tracebuf_used

static char *
_nc_trace_alloc(int bufnum, size_t want)
{
    char *result = 0;

    if (bufnum >= 0) {
	if ((size_t) (bufnum + 1) > MySize) {
	    size_t need = (bufnum + 1) * 2;
	    if ((MyList = typeRealloc(TRACEBUF, need, MyList)) != 0) {
		while (need > MySize)
		    MyList[MySize++].text = 0;
	    }
	}

	if (MyList != 0) {
	    if (MyList[bufnum].text == 0
		|| want > MyList[bufnum].size) {
		MyList[bufnum].text = typeRealloc(char, want, MyList[bufnum].text);
		if (MyList[bufnum].text != 0)
		    MyList[bufnum].size = want;
	    }
	    result = MyList[bufnum].text;
	}
    }
#if NO_LEAKS
    else {
	if (MySize) {
	    if (MyList) {
		while (MySize--) {
		    if (MyList[MySize].text != 0) {
			free(MyList[MySize].text);
		    }
		}
		free(MyList);
		MyList = 0;
	    }
	    MySize = 0;
	}
    }
#endif
    return result;
}

/*
 * (re)Allocate a buffer big enough for the caller's wants.
 */
NCURSES_EXPORT(char *)
_nc_trace_buf(int bufnum, size_t want)
{
    char *result = _nc_trace_alloc(bufnum, want);
    if (result != 0)
	*result = '\0';
    return result;
}

/*
 * Append a new string to an existing buffer.
 */
NCURSES_EXPORT(char *)
_nc_trace_bufcat(int bufnum, const char *value)
{
    char *buffer = _nc_trace_alloc(bufnum, 0);
    if (buffer != 0) {
	size_t have = strlen(buffer);

	buffer = _nc_trace_alloc(bufnum, 1 + have + strlen(value));
	if (buffer != 0)
	    (void) strcpy(buffer + have, value);

    }
    return buffer;
}