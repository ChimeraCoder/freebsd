
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1998                        *
 ****************************************************************************/

/*
 * Wrapper for malloc/realloc.  Standard implementations allow realloc with
 * a null pointer, but older libraries may not (e.g., SunOS).
 *
 * Also if realloc fails, we discard the old memory to avoid leaks.
 */

#include <curses.priv.h>

MODULE_ID("$Id: doalloc.c,v 1.8 2002/08/31 21:48:11 Philippe.Blain Exp $")

NCURSES_EXPORT(void *)
_nc_doalloc(void *oldp, size_t amount)
{
    void *newp;

    if (oldp != 0) {
	if ((newp = realloc(oldp, amount)) == 0) {
	    free(oldp);
	    errno = ENOMEM;	/* just in case 'free' reset */
	}
    } else {
	newp = malloc(amount);
    }
    return newp;
}

#if !HAVE_STRDUP
NCURSES_EXPORT(char *)
_nc_strdup(const char *src)
{
    char *dst;
    if (src != 0) {
	dst = typeMalloc(char, strlen(src) + 1);
	if (dst != 0) {
	    (void) strcpy(dst, src);
	}
    } else {
	dst = 0;
    }
    return dst;
}
#endif