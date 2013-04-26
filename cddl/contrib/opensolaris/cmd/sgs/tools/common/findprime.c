
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/types.h>
#include <sgs.h>

/*
 * function that will find a prime'ish number.  Usefull for
 * hashbuckets and related things.
 */
uint_t
findprime(uint_t count)
{
	uint_t	h, f;

	if (count <= 3)
		return (3);


	/*
	 * Check to see if divisible by two, if so
	 * increment.
	 */
	if ((count & 0x1) == 0)
		count++;

	for (h = count, f = 2; f * f <= h; f++)
		if ((h % f) == 0)
			h += f = 1;
	return (h);
}