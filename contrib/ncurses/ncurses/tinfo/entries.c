
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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

#include <curses.priv.h>

#include <ctype.h>

#include <tic.h>
#include <term_entry.h>

MODULE_ID("$Id: entries.c,v 1.8 2008/09/27 13:11:10 tom Exp $")

/****************************************************************************
 *
 * Entry queue handling
 *
 ****************************************************************************/
/*
 *  The entry list is a doubly linked list with NULLs terminating the lists:
 *
 *	  ---------   ---------   ---------
 *	  |       |   |       |   |       |   offset
 *        |-------|   |-------|   |-------|
 *	  |   ----+-->|   ----+-->|  NULL |   next
 *	  |-------|   |-------|   |-------|
 *	  |  NULL |<--+----   |<--+----   |   last
 *	  ---------   ---------   ---------
 *	      ^                       ^
 *	      |                       |
 *	      |                       |
 *	   _nc_head                _nc_tail
 */

NCURSES_EXPORT_VAR(ENTRY *) _nc_head = 0;
NCURSES_EXPORT_VAR(ENTRY *) _nc_tail = 0;

NCURSES_EXPORT(void)
_nc_free_entry(ENTRY * headp, TERMTYPE *tterm)
/* free the allocated storage consumed by the given list entry */
{
    ENTRY *ep;

    if ((ep = _nc_delink_entry(headp, tterm)) != 0) {
	free(ep);
    }
}

NCURSES_EXPORT(void)
_nc_free_entries(ENTRY * headp)
/* free the allocated storage consumed by list entries */
{
    (void) headp;		/* unused - _nc_head is altered here! */

    while (_nc_head != 0) {
	_nc_free_termtype(&(_nc_head->tterm));
    }
}

NCURSES_EXPORT(ENTRY *)
_nc_delink_entry(ENTRY * headp, TERMTYPE *tterm)
/* delink the allocated storage for the given list entry */
{
    ENTRY *ep, *last;

    for (last = 0, ep = headp; ep != 0; last = ep, ep = ep->next) {
	if (&(ep->tterm) == tterm) {
	    if (last != 0) {
		last->next = ep->next;
	    }
	    if (ep == _nc_head) {
		_nc_head = ep->next;
	    }
	    if (ep == _nc_tail) {
		_nc_tail = last;
	    }
	    break;
	}
    }
    return ep;
}

NCURSES_EXPORT(void)
_nc_leaks_tinfo(void)
{
#if NO_LEAKS
    char *s;
#endif

    T((T_CALLED("_nc_free_tinfo()")));
#if NO_LEAKS
    _nc_free_tparm();
    _nc_tgetent_leaks();
    _nc_free_entries(_nc_head);
    _nc_get_type(0);
    _nc_first_name(0);
    _nc_keyname_leaks();
#if BROKEN_LINKER || USE_REENTRANT
    _nc_names_leaks();
    _nc_codes_leaks();
    FreeIfNeeded(_nc_prescreen.real_acs_map);
#endif

    if ((s = _nc_home_terminfo()) != 0)
	free(s);
#endif /* NO_LEAKS */
    returnVoid;
}

#if NO_LEAKS
NCURSES_EXPORT(void)
_nc_free_tinfo(int code)
{
    _nc_leaks_tinfo();
    exit(code);
}
#endif