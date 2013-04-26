
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)unctime.c	8.2 (Berkeley) 6/14/94";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <time.h>

/*
 * Convert a ctime(3) format string into a system format date.
 * Return the date thus calculated.
 *
 * Return -1 if the string is not in ctime format.
 */
time_t
unctime(char *str)
{
	struct tm then;

	str = strptime(str, "%a %b %e %T %Y", &then);
	if (str == NULL || (*str != '\n' && *str != '\0'))
		return ((time_t)-1);
	then.tm_isdst = -1;
	return (mktime(&then));
}