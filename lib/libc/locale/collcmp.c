
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

#include <string.h>
#include <xlocale.h>
#include "collate.h"

/*
 * Compare two characters using collate
 */

int __collate_range_cmp(struct xlocale_collate *table, int c1, int c2)
{
	static char s1[2], s2[2];

	s1[0] = c1;
	s2[0] = c2;
	struct _xlocale l = {{0}};
	l.components[XLC_COLLATE] = (struct xlocale_component *)table;
	return (strcoll_l(s1, s2, &l));
}