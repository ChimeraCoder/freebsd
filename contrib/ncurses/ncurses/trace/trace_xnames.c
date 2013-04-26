
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
 *	trace_xnames.c - Tracing/Debugging buffers (TERMTYPE extended names)
 */

#include <curses.priv.h>
#include <term_entry.h>

MODULE_ID("$Id: trace_xnames.c,v 1.5 2000/12/10 03:02:45 tom Exp $")

NCURSES_EXPORT(void)
_nc_trace_xnames(TERMTYPE * tp GCC_UNUSED)
{
#ifdef TRACE
#if NCURSES_XNAMES
    int limit = tp->ext_Booleans + tp->ext_Numbers + tp->ext_Strings;
    int n, m;
    if (limit) {
	int begin_num = tp->ext_Booleans;
	int begin_str = tp->ext_Booleans + tp->ext_Numbers;

	_tracef("extended names (%s) %d = %d+%d+%d of %d+%d+%d",
		tp->term_names,
		limit,
		tp->ext_Booleans, tp->ext_Numbers, tp->ext_Strings,
		tp->num_Booleans, tp->num_Numbers, tp->num_Strings);
	for (n = 0; n < limit; n++) {
	    if ((m = n - begin_str) >= 0) {
		_tracef("[%d] %s = %s", n,
			tp->ext_Names[n],
			_nc_visbuf(tp->Strings[tp->num_Strings + m - tp->ext_Strings]));
	    } else if ((m = n - begin_num) >= 0) {
		_tracef("[%d] %s = %d (num)", n,
			tp->ext_Names[n],
			tp->Numbers[tp->num_Numbers + m - tp->ext_Numbers]);
	    } else {
		_tracef("[%d] %s = %d (bool)", n,
			tp->ext_Names[n],
			tp->Booleans[tp->num_Booleans + n - tp->ext_Booleans]);
	    }
	}
    }
#endif
#endif
}