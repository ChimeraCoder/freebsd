
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
 *  Author: Thomas E. Dickey 2002-on                                        *
 ****************************************************************************/

/*
**	lib_get_wch.c
**
**	The routine get_wch().
**
*/

#include <curses.priv.h>
#include <ctype.h>

MODULE_ID("$Id: lib_get_wch.c,v 1.17 2008/08/16 19:22:55 tom Exp $")

#if HAVE_MBTOWC && HAVE_MBLEN
#define reset_mbytes(state) mblen(NULL, 0), mbtowc(NULL, NULL, 0)
#define count_mbytes(buffer,length,state) mblen(buffer,length)
#define check_mbytes(wch,buffer,length,state) \
	(int) mbtowc(&wch, buffer, length)
#define state_unused
#elif HAVE_MBRTOWC && HAVE_MBRLEN
#define reset_mbytes(state) init_mb(state)
#define count_mbytes(buffer,length,state) mbrlen(buffer,length,&state)
#define check_mbytes(wch,buffer,length,state) \
	(int) mbrtowc(&wch, buffer, length, &state)
#else
make an error
#endif

NCURSES_EXPORT(int)
wget_wch(WINDOW *win, wint_t *result)
{
    SCREEN *sp;
    int code;
    char buffer[(MB_LEN_MAX * 9) + 1];	/* allow some redundant shifts */
    int status;
    size_t count = 0;
    unsigned long value;
    wchar_t wch;
#ifndef state_unused
    mbstate_t state;
#endif

    T((T_CALLED("wget_wch(%p)"), win));

    /*
     * We can get a stream of single-byte characters and KEY_xxx codes from
     * _nc_wgetch(), while we want to return a wide character or KEY_xxx code.
     */
    _nc_lock_global(curses);
    sp = _nc_screen_of(win);
    if (sp != 0) {
	for (;;) {
	    T(("reading %d of %d", (int) count + 1, (int) sizeof(buffer)));
	    code = _nc_wgetch(win, &value, TRUE EVENTLIST_2nd((_nc_eventlist
							       *) 0));
	    if (code == ERR) {
		break;
	    } else if (code == KEY_CODE_YES) {
		/*
		 * If we were processing an incomplete multibyte character,
		 * return an error since we have a KEY_xxx code which
		 * interrupts it.  For some cases, we could improve this by
		 * writing a new version of lib_getch.c(!), but it is not clear
		 * whether the improvement would be worth the effort.
		 */
		if (count != 0) {
		    _nc_ungetch(sp, (int) value);
		    code = ERR;
		}
		break;
	    } else if (count + 1 >= sizeof(buffer)) {
		_nc_ungetch(sp, (int) value);
		code = ERR;
		break;
	    } else {
		buffer[count++] = (char) UChar(value);
		reset_mbytes(state);
		status = count_mbytes(buffer, count, state);
		if (status >= 0) {
		    reset_mbytes(state);
		    if (check_mbytes(wch, buffer, count, state) != status) {
			code = ERR;	/* the two calls should match */
			_nc_ungetch(sp, (int) value);
		    }
		    value = wch;
		    break;
		}
	    }
	}
    } else {
	code = ERR;
    }
    *result = value;
    _nc_unlock_global(curses);
    T(("result %#lo", value));
    returnCode(code);
}