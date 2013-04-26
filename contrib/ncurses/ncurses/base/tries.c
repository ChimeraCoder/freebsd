
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1997                        *
 ****************************************************************************/

/*
**	tries.c
**
**	Functions to manage the tree of partial-completions for keycodes.
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: tries.c,v 1.27 2008/08/16 19:22:55 tom Exp $")

/*
 * Expand a keycode into the string that it corresponds to, returning null if
 * no match was found, otherwise allocating a string of the result.
 */
NCURSES_EXPORT(char *)
_nc_expand_try(TRIES * tree, unsigned code, int *count, size_t len)
{
    TRIES *ptr = tree;
    char *result = 0;

    if (code != 0) {
	while (ptr != 0) {
	    if ((result = _nc_expand_try(ptr->child, code, count, len + 1))
		!= 0) {
		break;
	    }
	    if (ptr->value == code) {
		*count -= 1;
		if (*count == -1) {
		    result = typeCalloc(char, len + 2);
		    break;
		}
	    }
	    ptr = ptr->sibling;
	}
    }
    if (result != 0) {
	if (ptr != 0 && (result[len] = (char) ptr->ch) == 0)
	    *((unsigned char *) (result + len)) = 128;
#ifdef TRACE
	if (len == 0 && USE_TRACEF(TRACE_MAXIMUM)) {
	    _tracef("expand_key %s %s", _nc_tracechar(SP, code), _nc_visbuf(result));
	    _nc_unlock_global(tracef);
	}
#endif
    }
    return result;
}

/*
 * Remove a code from the specified tree, freeing the unused nodes.  Returns
 * true if the code was found/removed.
 */
NCURSES_EXPORT(int)
_nc_remove_key(TRIES ** tree, unsigned code)
{
    T((T_CALLED("_nc_remove_key(%p,%d)"), tree, code));

    if (code == 0)
	returnCode(FALSE);

    while (*tree != 0) {
	if (_nc_remove_key(&(*tree)->child, code)) {
	    returnCode(TRUE);
	}
	if ((*tree)->value == code) {
	    if ((*tree)->child) {
		/* don't cut the whole sub-tree */
		(*tree)->value = 0;
	    } else {
		TRIES *to_free = *tree;
		*tree = (*tree)->sibling;
		free(to_free);
	    }
	    returnCode(TRUE);
	}
	tree = &(*tree)->sibling;
    }
    returnCode(FALSE);
}

/*
 * Remove a string from the specified tree, freeing the unused nodes.  Returns
 * true if the string was found/removed.
 */
NCURSES_EXPORT(int)
_nc_remove_string(TRIES ** tree, const char *string)
{
    T((T_CALLED("_nc_remove_string(%p,%s)"), tree, _nc_visbuf(string)));

    if (string == 0 || *string == 0)
	returnCode(FALSE);

    while (*tree != 0) {
	if (UChar((*tree)->ch) == UChar(*string)) {
	    if (string[1] != 0)
		returnCode(_nc_remove_string(&(*tree)->child, string + 1));
	    if ((*tree)->child == 0) {
		TRIES *to_free = *tree;
		*tree = (*tree)->sibling;
		free(to_free);
		returnCode(TRUE);
	    } else {
		returnCode(FALSE);
	    }
	}
	tree = &(*tree)->sibling;
    }
    returnCode(FALSE);
}