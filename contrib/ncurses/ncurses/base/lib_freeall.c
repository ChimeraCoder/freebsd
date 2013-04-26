
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
 *  Author: Thomas E. Dickey                    1996-on                     *
 ****************************************************************************/

#include <curses.priv.h>
#include <term_entry.h>
#include <tic.h>

#if HAVE_NC_FREEALL

#if HAVE_LIBDBMALLOC
extern int malloc_errfd;	/* FIXME */
#endif

MODULE_ID("$Id: lib_freeall.c,v 1.54 2008/09/27 13:09:57 tom Exp $")

/*
 * Free all ncurses data.  This is used for testing only (there's no practical
 * use for it as an extension).
 */
NCURSES_EXPORT(void)
_nc_freeall(void)
{
    WINDOWLIST *p, *q;
    static va_list empty_va;

    T((T_CALLED("_nc_freeall()")));
#if NO_LEAKS
    if (SP != 0) {
	if (SP->_oldnum_list != 0) {
	    FreeAndNull(SP->_oldnum_list);
	}
	if (SP->_panelHook.destroy != 0) {
	    SP->_panelHook.destroy(SP->_panelHook.stdscr_pseudo_panel);
	}
    }
#endif
    if (SP != 0) {
	_nc_lock_global(curses);

	while (_nc_windows != 0) {
	    bool deleted = FALSE;

	    /* Delete only windows that're not a parent */
	    for (each_window(p)) {
		bool found = FALSE;

		for (each_window(q)) {
		    if ((p != q)
			&& (q->win._flags & _SUBWIN)
			&& (&(p->win) == q->win._parent)) {
			found = TRUE;
			break;
		    }
		}

		if (!found) {
		    if (delwin(&(p->win)) != ERR)
			deleted = TRUE;
		    break;
		}
	    }

	    /*
	     * Don't continue to loop if the list is trashed.
	     */
	    if (!deleted)
		break;
	}
	delscreen(SP);
	_nc_unlock_global(curses);
    }
    if (cur_term != 0)
	del_curterm(cur_term);

    (void) _nc_printf_string(0, empty_va);
#ifdef TRACE
    (void) _nc_trace_buf(-1, 0);
#endif
#if USE_WIDEC_SUPPORT
    FreeIfNeeded(_nc_wacs);
#endif
    _nc_leaks_tinfo();

#if HAVE_LIBDBMALLOC
    malloc_dump(malloc_errfd);
#elif HAVE_LIBDMALLOC
#elif HAVE_LIBMPATROL
    __mp_summary();
#elif HAVE_PURIFY
    purify_all_inuse();
#endif
    returnVoid;
}

NCURSES_EXPORT(void)
_nc_free_and_exit(int code)
{
    char *last_setbuf = (SP != 0) ? SP->_setbuf : 0;

    _nc_freeall();
#ifdef TRACE
    trace(0);			/* close trace file, freeing its setbuf */
    {
	static va_list fake;
	free(_nc_varargs("?", fake));
    }
#endif
    fclose(stdout);
    FreeIfNeeded(last_setbuf);
    exit(code);
}

#else
NCURSES_EXPORT(void)
_nc_freeall(void)
{
}

NCURSES_EXPORT(void)
_nc_free_and_exit(int code)
{
    if (SP)
	delscreen(SP);
    if (cur_term != 0)
	del_curterm(cur_term);
    exit(code);
}
#endif