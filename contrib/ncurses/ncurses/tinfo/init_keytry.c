
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

#include <curses.priv.h>

#include <term.h>
/* keypad_xmit, keypad_local, meta_on, meta_off */
/* cursor_visible,cursor_normal,cursor_invisible */

#include <tic.h>		/* struct tinfo_fkeys */

#include <term_entry.h>

MODULE_ID("$Id: init_keytry.c,v 1.12 2008/05/24 21:44:51 tom Exp $")

/*
**      _nc_init_keytry()
**
**      Construct the try for the current terminal's keypad keys.
**
*/

/*
 * Internal entrypoints use SCREEN* parameter to obtain capabilities rather
 * than cur_term.
 */
#undef CUR
#define CUR (sp->_term)->type.

#if	BROKEN_LINKER
#undef	_nc_tinfo_fkeys
#endif

/* LINT_PREPRO
#if 0*/
#include <init_keytry.h>
/* LINT_PREPRO
#endif*/

#if	BROKEN_LINKER
const struct tinfo_fkeys *
_nc_tinfo_fkeysf(void)
{
    return _nc_tinfo_fkeys;
}
#endif

NCURSES_EXPORT(void)
_nc_init_keytry(SCREEN *sp)
{
    size_t n;

    /* The sp->_keytry value is initialized in newterm(), where the sp
     * structure is created, because we can not tell where keypad() or
     * mouse_activate() (which will call keyok()) are first called.
     */

    if (sp != 0) {
	for (n = 0; _nc_tinfo_fkeys[n].code; n++) {
	    if (_nc_tinfo_fkeys[n].offset < STRCOUNT) {
		(void) _nc_add_to_try(&(sp->_keytry),
				      CUR Strings[_nc_tinfo_fkeys[n].offset],
				      _nc_tinfo_fkeys[n].code);
	    }
	}
#if NCURSES_XNAMES
	/*
	 * Add any of the extended strings to the tries if their name begins
	 * with 'k', i.e., they follow the convention of other terminfo key
	 * names.
	 */
	{
	    TERMTYPE *tp = &(sp->_term->type);
	    for (n = STRCOUNT; n < NUM_STRINGS(tp); ++n) {
		const char *name = ExtStrname(tp, n, strnames);
		char *value = tp->Strings[n];
		if (name != 0
		    && *name == 'k'
		    && value != 0
		    && key_defined(value) == 0) {
		    (void) _nc_add_to_try(&(sp->_keytry),
					  value,
					  n - STRCOUNT + KEY_MAX);
		}
	    }
	}
#endif
#ifdef TRACE
	_nc_trace_tries(sp->_keytry);
#endif
    }
}