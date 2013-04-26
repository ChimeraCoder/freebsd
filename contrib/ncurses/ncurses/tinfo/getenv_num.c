
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
 *	getenv_num.c -- obtain a number from the environment
 */

#include <curses.priv.h>

MODULE_ID("$Id: getenv_num.c,v 1.3 2000/12/10 02:55:07 tom Exp $")

NCURSES_EXPORT(int)
_nc_getenv_num(const char *name)
{
    char *dst = 0;
    char *src = getenv(name);
    long value;

    if ((src == 0)
	|| (value = strtol(src, &dst, 0)) < 0
	|| (dst == src)
	|| (*dst != '\0')
	|| (int) value < value)
	value = -1;

    return (int) value;
}