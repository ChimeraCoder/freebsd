
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
 *  Author: Thomas E. Dickey 1996-on                                        *
 *     and: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/*
 *	lib_tracedmp.c - Tracing/Debugging routines
 */

#include <curses.priv.h>
#include <ctype.h>

MODULE_ID("$Id: lib_tracedmp.c,v 1.31 2008/08/16 19:30:56 tom Exp $")

#ifdef TRACE

#define my_buffer _nc_globals.tracedmp_buf
#define my_length _nc_globals.tracedmp_used

NCURSES_EXPORT(void)
_tracedump(const char *name, WINDOW *win)
{
    int i, j, n, width;

    /* compute narrowest possible display width */
    for (width = i = 0; i <= win->_maxy; ++i) {
	n = 0;
	for (j = 0; j <= win->_maxx; ++j) {
	    if (CharOf(win->_line[i].text[j]) != L(' ')
		|| AttrOf(win->_line[i].text[j]) != A_NORMAL
		|| GetPair(win->_line[i].text[j]) != 0) {
		n = j;
	    }
	}

	if (n > width)
	    width = n;
    }
    if (width < win->_maxx)
	++width;
    if (++width + 1 > (int) my_length) {
	my_length = 2 * (width + 1);
	my_buffer = typeRealloc(char, my_length, my_buffer);
    }

    for (n = 0; n <= win->_maxy; ++n) {
	char *ep = my_buffer;
	bool haveattrs, havecolors;

	/*
	 * Dump A_CHARTEXT part.  It is more important to make the grid line up
	 * in the trace file than to represent control- and wide-characters, so
	 * we map those to '.' and '?' respectively.
	 */
	for (j = 0; j < width; ++j) {
	    chtype test = CharOf(win->_line[n].text[j]);
	    ep[j] = (char) ((UChar(test) == test
#if USE_WIDEC_SUPPORT
			     && (win->_line[n].text[j].chars[1] == 0)
#endif
			    )
			    ? (iscntrl(UChar(test))
			       ? '.'
			       : UChar(test))
			    : '?');
	}
	ep[j] = '\0';
	_tracef("%s[%2d] %3ld%3ld ='%s'",
		name, n,
		(long) win->_line[n].firstchar,
		(long) win->_line[n].lastchar,
		ep);

	/* if there are multi-column characters on the line, print them now */
	if_WIDEC({
	    bool multicolumn = FALSE;
	    for (j = 0; j < width; ++j)
		if (WidecExt(win->_line[n].text[j]) != 0) {
		    multicolumn = TRUE;
		    break;
		}
	    if (multicolumn) {
		ep = my_buffer;
		for (j = 0; j < width; ++j) {
		    int test = WidecExt(win->_line[n].text[j]);
		    if (test) {
			ep[j] = (char) (test + '0');
		    } else {
			ep[j] = ' ';
		    }
		}
		ep[j] = '\0';
		_tracef("%*s[%2d]%*s='%s'", (int) strlen(name),
			"widec", n, 8, " ", my_buffer);
	    }
	});

	/* dump A_COLOR part, will screw up if there are more than 96 */
	havecolors = FALSE;
	for (j = 0; j < width; ++j)
	    if (GetPair(win->_line[n].text[j]) != 0) {
		havecolors = TRUE;
		break;
	    }
	if (havecolors) {
	    ep = my_buffer;
	    for (j = 0; j < width; ++j) {
		int pair = GetPair(win->_line[n].text[j]);
		if (pair >= 52)
		    ep[j] = '?';
		else if (pair >= 36)
		    ep[j] = (char) (pair + 'A');
		else if (pair >= 10)
		    ep[j] = (char) (pair + 'a');
		else if (pair >= 1)
		    ep[j] = (char) (pair + '0');
		else
		    ep[j] = ' ';
	    }
	    ep[j] = '\0';
	    _tracef("%*s[%2d]%*s='%s'", (int) strlen(name),
		    "colors", n, 8, " ", my_buffer);
	}

	for (i = 0; i < 4; ++i) {
	    const char *hex = " 123456789ABCDEF";
	    attr_t mask = (0xf << ((i + 4) * 4));

	    haveattrs = FALSE;
	    for (j = 0; j < width; ++j)
		if (AttrOf(win->_line[n].text[j]) & mask) {
		    haveattrs = TRUE;
		    break;
		}
	    if (haveattrs) {
		ep = my_buffer;
		for (j = 0; j < width; ++j)
		    ep[j] = hex[(AttrOf(win->_line[n].text[j]) & mask) >>
				((i + 4) * 4)];
		ep[j] = '\0';
		_tracef("%*s%d[%2d]%*s='%s'", (int) strlen(name) -
			1, "attrs", i, n, 8, " ", my_buffer);
	    }
	}
    }
#if NO_LEAKS
    free(my_buffer);
    my_buffer = 0;
    my_length = 0;
#endif
}

#else
EMPTY_MODULE(_nc_lib_tracedmp)
#endif /* TRACE */