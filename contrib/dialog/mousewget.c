
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

#include <dialog.h>
#include <dlg_keys.h>

static int
mouse_wgetch(WINDOW *win, int *fkey, bool ignore_errs)
{
    int mouse_err = FALSE;
    int key;

    do {

	key = dlg_getc(win, fkey);

#if USE_MOUSE

	mouse_err = FALSE;
	if (fkey && (key == KEY_MOUSE)) {
	    MEVENT event;
	    mseRegion *p;

	    if (getmouse(&event) != ERR) {
		if ((p = dlg_mouse_region(event.y, event.x)) != 0) {
		    key = DLGK_MOUSE(p->code);
		} else if ((p = dlg_mouse_bigregion(event.y, event.x)) != 0) {
		    int x = event.x - p->x;
		    int y = event.y - p->y;
		    int row = (p->X - p->x) / p->step_x;

		    key = -(p->code);
		    switch (p->mode) {
		    case 1:	/* index by lines */
			key += y;
			break;
		    case 2:	/* index by columns */
			key += (x / p->step_x);
			break;
		    default:
		    case 3:	/* index by cells */
			key += (x / p->step_x) + (y * row);
			break;
		    }
		} else {
		    (void) beep();
		    mouse_err = TRUE;
		}
	    } else {
		(void) beep();
		mouse_err = TRUE;
	    }
	}
#endif

    } while (ignore_errs && mouse_err);

    return key;
}

int
dlg_mouse_wgetch(WINDOW *win, int *fkey)
{
    return mouse_wgetch(win, fkey, TRUE);
}

int
dlg_mouse_wgetch_nowait(WINDOW *win, int *fkey)
{
    return mouse_wgetch(win, fkey, FALSE);
}