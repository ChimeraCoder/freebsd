
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
 *  Author: Thomas E. Dickey            1998-on                             *
 ****************************************************************************/

/*
**	add_tries.c
**
**	Add keycode/string to tries-tree.
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: add_tries.c,v 1.8 2006/12/30 23:15:26 tom Exp $")

#define SET_TRY(dst,src) if ((dst->ch = *src++) == 128) dst->ch = '\0'
#define CMP_TRY(a,b) ((a)? (a == b) : (b == 128))

NCURSES_EXPORT(int)
_nc_add_to_try(TRIES ** tree, const char *str, unsigned code)
{
    TRIES *ptr, *savedptr;
    unsigned const char *txt = (unsigned const char *) str;

    T((T_CALLED("_nc_add_to_try(%p, %s, %u)"), *tree, _nc_visbuf(str), code));
    if (txt == 0 || *txt == '\0' || code == 0)
	returnCode(ERR);

    if ((*tree) != 0) {
	ptr = savedptr = (*tree);

	for (;;) {
	    unsigned char cmp = *txt;

	    while (!CMP_TRY(ptr->ch, cmp)
		   && ptr->sibling != 0)
		ptr = ptr->sibling;

	    if (CMP_TRY(ptr->ch, cmp)) {
		if (*(++txt) == '\0') {
		    ptr->value = code;
		    returnCode(OK);
		}
		if (ptr->child != 0)
		    ptr = ptr->child;
		else
		    break;
	    } else {
		if ((ptr->sibling = typeCalloc(TRIES, 1)) == 0) {
		    returnCode(ERR);
		}

		savedptr = ptr = ptr->sibling;
		SET_TRY(ptr, txt);
		ptr->value = 0;

		break;
	    }
	}			/* end for (;;) */
    } else {			/* (*tree) == 0 :: First sequence to be added */
	savedptr = ptr = (*tree) = typeCalloc(TRIES, 1);

	if (ptr == 0) {
	    returnCode(ERR);
	}

	SET_TRY(ptr, txt);
	ptr->value = 0;
    }

    /* at this point, we are adding to the try.  ptr->child == 0 */

    while (*txt) {
	ptr->child = typeCalloc(TRIES, 1);

	ptr = ptr->child;

	if (ptr == 0) {
	    while ((ptr = savedptr) != 0) {
		savedptr = ptr->child;
		free(ptr);
	    }
	    returnCode(ERR);
	}

	SET_TRY(ptr, txt);
	ptr->value = 0;
    }

    ptr->value = code;
    returnCode(OK);
}