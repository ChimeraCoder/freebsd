
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

static int
test_str( const char *str, const struct eui64 *eui)
{
	struct eui64	e;
	char		buf[EUI64_SIZ];
	static int	test = 0;

	test++;

	if (eui64_aton(str, &e) != 0 &&
	    memcmp(&e, &eui, sizeof(struct eui64)) != 0) {
		printf("not ok %d - : eui64_aton(%s)\n", test, str);
		eui64_ntoa(&e, buf, sizeof(buf));
		printf("# got: %s\n", buf);
		return (0);
	} else {
		printf("ok %d - eui64_aton(%s)\n", test, str);
		return (1);
	}

}

int
main(int argc, char **argv)
{

	printf("1..5\n");

	test_str(test_eui64_id_ascii, &test_eui64_id);
	test_str(test_eui64_id_colon_ascii, &test_eui64_id);
	test_str(test_eui64_mac_ascii, &test_eui64_eui48);
	test_str(test_eui64_mac_colon_ascii, &test_eui64_eui48);
	test_str(test_eui64_hex_ascii, &test_eui64_id);

	return (0);
}