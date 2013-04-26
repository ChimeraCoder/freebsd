
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

#include <sys/types.h>
#include <sys/eui64.h>
#include <stdio.h>
#include <string.h>

#include "test-eui64.h"

int
main(int argc, char **argv)
{
	char a[EUI64_SIZ];

	printf("1..1\n");

	if (eui64_ntoa(&test_eui64_id, a, sizeof(a)) == 0 &&
	    strcmp(a, test_eui64_id_ascii) == 0) {
		printf("ok 1 - eui64_ntoa\n");
		return (0);
	}
	printf("# a = '%s'\n", a);

	printf("not ok 1 - eui64_ntoa\n");
	return (0);
}