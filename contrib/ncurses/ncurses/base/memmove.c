
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

MODULE_ID("$Id: memmove.c,v 1.5 2007/08/11 17:12:43 tom Exp $")

/****************************************************************************
 *  Author: Thomas E. Dickey <dickey@clark.net> 1998                        *
 ****************************************************************************/

#if USE_MY_MEMMOVE
#define DST ((char *)s1)
#define SRC ((const char *)s2)
NCURSES_EXPORT(void *)
_nc_memmove(void *s1, const void *s2, size_t n)
{
    if (n != 0) {
	if ((DST + n > SRC) && (SRC + n > DST)) {
	    static char *bfr;
	    static size_t length;
	    register size_t j;
	    if (length < n) {
		length = (n * 3) / 2;
		bfr = typeRealloc(char, length, bfr);
	    }
	    for (j = 0; j < n; j++)
		bfr[j] = SRC[j];
	    s2 = bfr;
	}
	while (n-- != 0)
	    DST[n] = SRC[n];
    }
    return s1;
}
#else
extern
NCURSES_EXPORT(void)
_nc_memmove(void);		/* quiet's gcc warning */
NCURSES_EXPORT(void)
_nc_memmove(void)
{
}				/* nonempty for strict ANSI compilers */
#endif /* USE_MY_MEMMOVE */