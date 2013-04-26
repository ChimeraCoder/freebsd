
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1999                        *
 ****************************************************************************/
/*
 *	trace_tries.c - Tracing/Debugging buffers (keycode tries-trees)
 */

#include <curses.priv.h>

MODULE_ID("$Id: trace_tries.c,v 1.13 2008/08/03 15:43:30 tom Exp $")

#ifdef TRACE
#define my_buffer _nc_globals.tracetry_buf
#define my_length _nc_globals.tracetry_used

static void
recur_tries(TRIES * tree, unsigned level)
{
    if (level > my_length) {
	my_length = (level + 1) * 4;
	my_buffer = (unsigned char *) realloc(my_buffer, my_length);
    }

    while (tree != 0) {
	if ((my_buffer[level] = tree->ch) == 0)
	    my_buffer[level] = 128;
	my_buffer[level + 1] = 0;
	if (tree->value != 0) {
	    _tracef("%5d: %s (%s)", tree->value,
		    _nc_visbuf((char *) my_buffer), keyname(tree->value));
	}
	if (tree->child)
	    recur_tries(tree->child, level + 1);
	tree = tree->sibling;
    }
}

NCURSES_EXPORT(void)
_nc_trace_tries(TRIES * tree)
{
    my_buffer = typeMalloc(unsigned char, my_length = 80);
    _tracef("BEGIN tries %p", tree);
    recur_tries(tree, 0);
    _tracef(". . . tries %p", tree);
    free(my_buffer);
}

#else
EMPTY_MODULE(_nc_trace_tries)
#endif