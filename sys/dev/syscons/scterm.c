
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

#include "opt_syscons.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/consio.h>

#include <dev/syscons/syscons.h>

SET_DECLARE(scterm_set, sc_term_sw_t);

/* exported subroutines */

void
sc_move_cursor(scr_stat *scp, int x, int y)
{
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x >= scp->xsize)
		x = scp->xsize - 1;
	if (y >= scp->ysize)
		y = scp->ysize - 1;
	scp->xpos = x;
	scp->ypos = y;
	scp->cursor_pos = scp->ypos*scp->xsize + scp->xpos;
}

void
sc_clear_screen(scr_stat *scp)
{
	(*scp->tsw->te_clear)(scp);
	scp->cursor_oldpos = scp->cursor_pos;
	sc_remove_cutmarking(scp);
}

/* terminal emulator manager routines */

static LIST_HEAD(, sc_term_sw) sc_term_list = 
	LIST_HEAD_INITIALIZER(sc_term_list);

int
sc_term_add(sc_term_sw_t *sw)
{
	LIST_INSERT_HEAD(&sc_term_list, sw, link);
	return 0;
}

int
sc_term_remove(sc_term_sw_t *sw)
{
	LIST_REMOVE(sw, link);
	return 0;
}

sc_term_sw_t
*sc_term_match(char *name)
{
	sc_term_sw_t **list;
	sc_term_sw_t *p;

	if (!LIST_EMPTY(&sc_term_list)) {
		LIST_FOREACH(p, &sc_term_list, link) {
			if ((strcmp(name, p->te_name) == 0)
			    || (strcmp(name, "*") == 0)) {
				return p;
			}
		}
	} else {
		SET_FOREACH(list, scterm_set) {
			p = *list;
			if ((strcmp(name, p->te_name) == 0)
			    || (strcmp(name, "*") == 0)) {
				return p;
			}
		}
	}

	return NULL;
}

sc_term_sw_t
*sc_term_match_by_number(int index)
{
	sc_term_sw_t *p;

	if (index <= 0)
		return NULL;
	LIST_FOREACH(p, &sc_term_list, link) {
		if (--index <= 0)
			return p;
	}

	return NULL;
}